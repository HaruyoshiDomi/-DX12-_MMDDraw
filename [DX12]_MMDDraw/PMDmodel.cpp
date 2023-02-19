#include "main.h"
#include "Render.h"
#include "VMDmotion.h"
#include "XMFLOAT_Helper.h"
#include "PMDmodel.h"
#include "App.h"
#include <sstream>
#include <array>

namespace
{
	///Z軸を特定の方向を向かす行列を返す
	///@param lookat 向かせたい方向ベクトル
	///@param up 上ベクトル
	///@param right 右ベクトル
	XMMATRIX LookAtMatrix(const XMVECTOR& lookat, const XMFLOAT3& up, const XMFLOAT3& right)
	{
		//向かせたい方向(z軸)
		XMVECTOR vz = lookat;

		//(向かせたい方向を向かせたとき)仮のy軸ベクトル
		XMVECTOR vy = XMVector3Normalize(XMLoadFloat3(&up));

		//(向かせたい方向を向かせたときの)y軸
		XMVECTOR vx = {};
		//vx = XMVector3Normalize(XMVector3Cross(vz, vx));
		vx = XMVector3Normalize(XMVector3Cross(vy, vz));
		vy = XMVector3Normalize(XMVector3Cross(vz, vx));

		///LookAtとupが同じ方向を向いてたらright基準で作り直す
		if (abs(XMVector3Dot(vy, vz).m128_f32[0]) == 1.0f) 
		{
			//仮のX方向を定義
			vx = XMVector3Normalize(XMLoadFloat3(&right));
			//向かせたい方向を向かせたときのY軸を計算
			vy = XMVector3Normalize(XMVector3Cross(vz, vx));
			//真のX軸を計算
			vx = XMVector3Normalize(XMVector3Cross(vy, vz));
		}
		XMMATRIX ret = XMMatrixIdentity();
		ret.r[0] = vx;
		ret.r[1] = vy;
		ret.r[2] = -vz;
		return ret;

	}

	///特定のベクトルを特定の方向に向けるための行列を返す
	///@param origin 特定のベクトル
	///@param lookat 向かせたい方向
	///@param up 上ベクトル
	///@param right 右ベクトル
	///@retval 特定のベクトルを特定の方向に向けるための行列
	XMMATRIX LookAtMatrix(const XMVECTOR& origin, const XMVECTOR& lookat, const XMFLOAT3& up, const XMFLOAT3& right) 
	{
		return XMMatrixTranspose(LookAtMatrix(origin, up, right)) *
			LookAtMatrix(lookat, up, right);
	}

	//ボーン種別
	enum class BoneType
	{
		Rotation,
		RotAndMove,
		IK,
		Undefined,
		IKChild,
		RotationChild,
		IKDestination,
		Invitible
	};
}

PMDmodel::PMDmodel(const char* filepath, Render& dx12) : Model(dx12)
{
	auto result = CreateRootSignature();
	result = CreateGraphicsPipeline();
	m_transform.world = XMMatrixIdentity();
	result = LoadPMDFile(filepath);
	result = CreateTransformView();
	result = CreateMaterialData();
	result = CreateMaterialAndTextureView();
	RecursiveMatrixMultiply(&m_boneNodetable["センター"], XMMatrixIdentity());
	std::copy(m_boneMatAndQuat.begin(), m_boneMatAndQuat.end(), m_mappeboneMatAndQuats + 1);

}

PMDmodel::~PMDmodel()
{
	delete m_motion;
}

void PMDmodel::Update(XMMATRIX& martrix)
{

	m_mappeboneMatAndQuats[0].boneMatrieces = martrix;
	if (m_motion)
	{
		if (m_motion->GetMotionFlag())
		{
			m_motion->UpdateMotion();
			RecursiveMatrixMultiply(&m_boneNodetable["センター"], XMMatrixIdentity());
		}
		//IKSolve(m_motion->GetFrameNo());

		std::copy(m_boneMatAndQuat.begin(), m_boneMatAndQuat.end(), m_mappeboneMatAndQuats + 1);

		m_motion->GetMorphNameFromFrame(m_morphsName, m_morphsWeight);

		for (auto mn : m_morphsName)
		{
			for (auto md : m_morphsData[mn])
			{
				m_vertices[m_morphsData["base"][md.skinIdx].skinIdx].position = m_morphsData["base"][md.skinIdx].pos + (md.pos * m_morphsWeight[mn]);
			}
		}

		PMDVertex* vertMap = nullptr;
		auto result = m_vb->Map(0, nullptr, (void**)&vertMap);
		std::copy(m_vertices.begin(), m_vertices.end(), vertMap);


	}
}

void PMDmodel::Draw()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_dx12.CommandList()->OMSetBlendFactor(blendFactor);
	

	m_dx12.CommandList()->SetGraphicsRootSignature(this->m_rootSignature.Get());
	m_dx12.CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_dx12.SetSceneView();

	m_dx12.CommandList()->IASetVertexBuffers(0, 1, &m_vbView);
	m_dx12.CommandList()->IASetIndexBuffer(&m_ibView);

	ID3D12DescriptorHeap* transheaps[] = { m_transformHeap.Get() };
	m_dx12.CommandList()->SetDescriptorHeaps(1, transheaps);
	m_dx12.CommandList()->SetGraphicsRootDescriptorTable(1, m_transformHeap->GetGPUDescriptorHandleForHeapStart());



	ID3D12DescriptorHeap* mdh[] = { m_materialHeap.Get() };
	//マテリアル
	m_dx12.CommandList()->SetDescriptorHeaps(1, mdh);
	auto materialH = m_materialHeap->GetGPUDescriptorHandleForHeapStart();
	unsigned int idxOffset = 0;
	m_dx12.CommandList()->SetPipelineState(this->m_edgepipeline.Get());

	auto cbvsrvIncSize = m_dx12.Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 5;
	for (auto& m : m_materials)
	{

		m_dx12.CommandList()->SetGraphicsRootDescriptorTable(2, materialH);
		m_dx12.CommandList()->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
		materialH.ptr += cbvsrvIncSize;
		idxOffset += m.indicesNum;
	}

	m_dx12.CommandList()->SetPipelineState(this->m_pipeline.Get());
	materialH = m_materialHeap->GetGPUDescriptorHandleForHeapStart();
	idxOffset = 0;

	for (auto& m : m_materials)
	{
		m_dx12.CommandList()->SetGraphicsRootDescriptorTable(2, materialH);
		//m_dx12.CommandList()->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
		m_dx12.CommandList()->DrawIndexedInstanced(m.indicesNum, 2, idxOffset, 0, 0);
		materialH.ptr += cbvsrvIncSize;
		idxOffset += m.indicesNum;
	}
	
}

HRESULT PMDmodel::CreateGraphicsPipeline()
{
	if (!m_VSBlob && !m_PSBlob)
	{
		auto result = CreateVS(L"PMDVS.cso");
		if (FAILED(result))
		{
			assert(0);
			return S_FALSE;
		}

		result = CreatePS(L"PMDPS.cso");
		if (FAILED(result))
		{
			assert(0);
			return S_FALSE;
		}
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = 
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "BONENO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};
	CD3DX12_PIPELINE_STATE_STREAM2(desc);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = desc.GraphicsDescV0();
	gpipeline.pRootSignature = m_rootSignature.Get();
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(m_VSBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(m_PSBlob.Get());

	for (int i = 0; i < _countof(gpipeline.BlendState.RenderTarget); ++i)
	{
		//見やすくするため変数化
		auto& rt = gpipeline.BlendState.RenderTarget[i];
		rt.BlendEnable = true;
		rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		rt.BlendOp = D3D12_BLEND_OP_ADD;
		rt.SrcBlendAlpha = D3D12_BLEND_ONE;
		rt.DestBlendAlpha = D3D12_BLEND_ONE;
		rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//三角形で構成
	gpipeline.InputLayout.pInputElementDescs = inputLayout;						//レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements = _countof(inputLayout);					//レイアウト配列数
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;						//0～1に正規化されたRGBA
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	auto result = m_dx12.Device()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&m_pipeline));
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
	}

	result = CreateVS(L"EdgeVertexShader.cso");
	if (FAILED(result))
	{
		assert(0);
		return S_FALSE;
	}

	result = CreatePS(L"EdgePixelShader.cso");
	if (FAILED(result))
	{
		assert(0);
		return S_FALSE;
	}

	gpipeline.VS = CD3DX12_SHADER_BYTECODE(m_VSBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(m_PSBlob.Get());
	gpipeline.DepthStencilState.FrontFace;
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	result = m_dx12.Device()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&m_edgepipeline));
	if (FAILED(result))
	{
		assert(SUCCEEDED(result));
	}

	return result;
}

HRESULT PMDmodel::CreateRootSignature()
{
	//レンジ
	CD3DX12_DESCRIPTOR_RANGE  descTblRanges[4] = {};				//テクスチャと定数の２つ
	descTblRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	//定数[b0](ビュープロジェクション用)
	descTblRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);	//定数[b1](ワールド、ボーン用)
	descTblRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);	//定数[b2](マテリアル用)
	descTblRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 0);	//テクスチャ４つ(基本とsphとspaとトゥーン)

	//ルートパラメータ
	CD3DX12_ROOT_PARAMETER rootParams[3] = {};
	rootParams[0].InitAsDescriptorTable(1, &descTblRanges[0]);		//ビュープロジェクション変換
	rootParams[1].InitAsDescriptorTable(1, &descTblRanges[1]);		//ワールド・ボーン変換
	rootParams[2].InitAsDescriptorTable(2, &descTblRanges[2]);		//マテリアル周り

	CD3DX12_STATIC_SAMPLER_DESC samplerDescs[2] = {};
	samplerDescs[0].Init(0);
	samplerDescs[1].Init(1, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Init(3, rootParams, 2, samplerDescs, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	auto result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return result;
	}
	result = m_dx12.Device()->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf()));
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return result;
	}
	return result;
}

HRESULT PMDmodel::CreateMaterialData()
{
	//マテリアルバッファを作成
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * m_materials.size());

	auto result = m_dx12.Device()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_materialBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return result;
	}

	//マップマテリアルにコピー
	char* mapMaterial = nullptr;
	result = m_materialBuff->Map(0, nullptr, (void**)&mapMaterial);
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return result;
	}
	for (auto& m : m_materials) 
	{
		*((MaterialForHlsl*)mapMaterial) = m.material;//データコピー
		mapMaterial += materialBuffSize;//次のアライメント位置まで進める
	}
	m_materialBuff->Unmap(0, nullptr);

	return S_OK;
}

HRESULT PMDmodel::CreateMaterialAndTextureView()
{

	D3D12_DESCRIPTOR_HEAP_DESC materialDescHeapDesc = {};
	materialDescHeapDesc.NumDescriptors = m_materials.size() * 5;//マテリアル数ぶん(定数1つ、テクスチャ3つ)
	materialDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	materialDescHeapDesc.NodeMask = 0;

	materialDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	auto result = m_dx12.Device()->CreateDescriptorHeap(&materialDescHeapDesc, IID_PPV_ARGS(m_materialHeap.ReleaseAndGetAddressOf()));//生成
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return result;
	}
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff) & ~0xff;
	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = m_materialBuff->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = materialBuffSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//後述
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1
	CD3DX12_CPU_DESCRIPTOR_HANDLE matDescHeapH(m_materialHeap->GetCPUDescriptorHandleForHeapStart());
	auto incSize = m_dx12.Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (int i = 0; i < m_materials.size(); ++i) 
	{
		//マテリアル固定バッファビュー
		m_dx12.Device()->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
		matDescHeapH.ptr += incSize;
		matCBVDesc.BufferLocation += materialBuffSize;
		if (m_textureResources[i])
		{
			srvDesc.Format = m_textureResources[i]->GetDesc().Format;
			m_dx12.Device()->CreateShaderResourceView(m_textureResources[i].Get(), &srvDesc, matDescHeapH);
		}
		else
		{
			srvDesc.Format = m_whiteTex->GetDesc().Format;
			m_dx12.Device()->CreateShaderResourceView(m_whiteTex.Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.Offset(incSize);

		if (m_sphResources[i])
		{
			srvDesc.Format = m_sphResources[i]->GetDesc().Format;
			m_dx12.Device()->CreateShaderResourceView(m_sphResources[i].Get(), &srvDesc, matDescHeapH);
		}
		else
		{
			srvDesc.Format = m_whiteTex->GetDesc().Format;
			m_dx12.Device()->CreateShaderResourceView(m_whiteTex.Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (m_spaResources[i])
		{
			srvDesc.Format = m_spaResources[i]->GetDesc().Format;
			m_dx12.Device()->CreateShaderResourceView(m_spaResources[i].Get(), &srvDesc, matDescHeapH);
		}
		else
		{
			srvDesc.Format = m_blackTex->GetDesc().Format;
			m_dx12.Device()->CreateShaderResourceView(m_blackTex.Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (m_toonResources[i])
		{
			srvDesc.Format = m_toonResources[i]->GetDesc().Format;
			m_dx12.Device()->CreateShaderResourceView(m_toonResources[i].Get(), &srvDesc, matDescHeapH);
		}
		else
		{
			srvDesc.Format = m_whiteTex->GetDesc().Format;
			m_dx12.Device()->CreateShaderResourceView(m_whiteTex.Get(), &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;
	}

	return S_OK;
}

HRESULT PMDmodel::CreateTransformView()
{
	//GPUバッファ作成
	auto buffSize = sizeof(MatAndQuat) * (1 + m_boneMatAndQuat.size());
	buffSize = (buffSize + 0xff) & ~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(buffSize);

	auto result = m_dx12.Device()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_transformBuff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return result;
	}

	//マップとコピー
	result = m_transformBuff->Map(0, nullptr, (void**)&m_mappeboneMatAndQuats);
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return result;
	}
	m_mappeboneMatAndQuats[0].boneMatrieces = m_transform.world;

	std::copy(m_boneMatAndQuat.begin(), m_boneMatAndQuat.end(), m_mappeboneMatAndQuats + 1);

	//ビューの作成
	D3D12_DESCRIPTOR_HEAP_DESC transformDescHeapDesc = {};
	transformDescHeapDesc.NumDescriptors = 1;//とりあえずワールドひとつ
	transformDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	transformDescHeapDesc.NodeMask = 0;

	transformDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	result = m_dx12.Device()->CreateDescriptorHeap(&transformDescHeapDesc, IID_PPV_ARGS(m_transformHeap.ReleaseAndGetAddressOf()));//生成
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return result;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_transformBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = buffSize;
	m_dx12.Device()->CreateConstantBufferView(&cbvDesc, m_transformHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}

HRESULT PMDmodel::LoadPMDFile(const char* path)
{
	//PMDヘッダー構造体
	struct PMDHeader
	{
		float vertion;		//例：00 00 80 3F == 1.00
		char modelname[20];
		char comment[256];
	};
	char signature[3] = {};		//シグネチャ
	PMDHeader pmdheader = {};
	FILE* fp;
	std::string strModelPath = path;
	auto err = fopen_s(&fp, strModelPath.c_str(), "rb");
	if (!fp)
	{
		assert(0);
		return ERROR_FILE_NOT_FOUND;
	}

	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);
	std::cout << pmdheader.modelname;
	m_name = (pmdheader.modelname);
	std::cout << pmdheader.comment << std::endl;
	unsigned int vertNum;//頂点数
	fread(&vertNum, sizeof(vertNum), 1, fp);

#pragma pack(1)
	struct PMDMaterial
	{
		XMFLOAT3 diffuse;
		float alfa;
		float specularity;
		XMFLOAT3 speculer;
		XMFLOAT3 ambient;
		unsigned char toonIdx;
		unsigned char edgeflag;

		unsigned int indicesNum;	//このマテリアルが割り当てられるインデックス数
		char texfilePath[20];
	};
#pragma pack()

	constexpr unsigned int pmdvertex_size = 38;//頂点1つあたりのサイズ
	m_vertices.resize(vertNum);//バッファ確保
	for (int i = 0; i < vertNum; ++i)
	{
		fread(&m_vertices[i].position, sizeof(m_vertices[i].position), 1, fp);
		fread(&m_vertices[i].normal, sizeof(m_vertices[i].normal), 1, fp);
		fread(&m_vertices[i].uv, sizeof(m_vertices[i].uv), 1, fp);
		fread(&m_vertices[i].boneNum[0], sizeof(WORD), 1, fp);
		fread(&m_vertices[i].boneNum[1], sizeof(WORD), 1, fp);
		fread(&m_vertices[i].weight, sizeof(m_vertices[i].weight), 1, fp);
		fread(&m_vertices[i].edgeFlg, sizeof(m_vertices[i].edgeFlg), 1, fp);
	}

	unsigned int indicesNum;//インデックス数
	fread(&indicesNum, sizeof(indicesNum), 1, fp);//

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_vertices.size() * sizeof(m_vertices[0]));

	//UPLOAD(確保は可能)
	auto result = m_dx12.Device()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_vb.ReleaseAndGetAddressOf()));

	//モーフ更新のためUnmapしない
	PMDVertex* vertMap = nullptr;
	result = m_vb->Map(0, nullptr, (void**)&vertMap);
	std::copy(m_vertices.begin(), m_vertices.end(), vertMap);


	m_vbView.BufferLocation = m_vb->GetGPUVirtualAddress();//バッファの仮想アドレス
	m_vbView.SizeInBytes = m_vertices.size() * sizeof(PMDVertex);//全バイト数
	m_vbView.StrideInBytes = sizeof(PMDVertex);//1頂点あたりのバイト数

	std::vector<unsigned short> indices(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);//一気に読み込み

	auto resDescBuf = CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]));


	result = m_dx12.Device()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDescBuf,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_ib.ReleaseAndGetAddressOf()));

	//作ったバッファにインデックスデータをコピー
	unsigned short* mappedIdx = nullptr;
	m_ib->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(indices.begin(), indices.end(), mappedIdx);
	m_ib->Unmap(0, nullptr);


	//インデックスバッファビューを作成
	m_ibView.BufferLocation = m_ib->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R16_UINT;
	m_ibView.SizeInBytes = indices.size() * sizeof(indices[0]);

	unsigned int materialNum;
	fread(&materialNum, sizeof(materialNum), 1, fp);
	m_materials.resize(materialNum);
	m_textureResources.resize(materialNum);
	m_sphResources.resize(materialNum);
	m_spaResources.resize(materialNum);
	m_toonResources.resize(materialNum);

	std::vector<PMDMaterial> pmdmaterials(materialNum);
	fread(pmdmaterials.data(), pmdmaterials.size() * sizeof(PMDMaterial), 1, fp);

	//コピー
	for (int i = 0; i < (int)pmdmaterials.size(); ++i)
	{
		m_materials[i].indicesNum = pmdmaterials[i].indicesNum;
		m_materials[i].material.diffuse = pmdmaterials[i].diffuse;
		m_materials[i].material.alpha = pmdmaterials[i].alfa;
		m_materials[i].material.specular = pmdmaterials[i].speculer;
		m_materials[i].material.specularity = pmdmaterials[i].specularity;
		m_materials[i].material.ambient = pmdmaterials[i].ambient;
		m_materials[i].material.edgeFlag = pmdmaterials[i].edgeflag;
		m_materials[i].additional.toonIdx = pmdmaterials[i].toonIdx;
	}

	for (int i = 0; i < (int)pmdmaterials.size(); ++i)
	{
		std::string toonFilePath = (pmdmaterials[i].texfilePath);
		std::string originfilename = {};
		char toonfilename[32];
	
		auto toonFilePathH = helper::SplitFileName(toonFilePath, '/');

		sprintf_s(toonfilename, "/%02d.bmp", pmdmaterials[i].toonIdx + 1);
		toonFilePath = helper::GetTexturePathFromModelAndTexPath(strModelPath, (toonFilePathH.first + toonfilename).c_str());
		m_toonResources[i] = m_dx12.GetTextureByPath(toonFilePath.c_str());
	
			
		

		if (strlen(pmdmaterials[i].texfilePath) == 0)
		{
			m_textureResources[i] = nullptr;
			continue;
		}

		std::string texfilename = pmdmaterials[i].texfilePath;
		std::string sphfileName = {};
		std::string spafileName = {};

		if (std::count(texfilename.begin(), texfilename.end(), '*') > 0)
		{
			//スプリッタがある
			auto namepair = helper::SplitFileName(texfilename);
			if (helper::GetExtension(namepair.first) == "sph"	)
			{
				sphfileName = namepair.first;
				texfilename = namepair.second;
			}
			else if (helper::GetExtension(namepair.first) == "spa")
			{
				spafileName = namepair.first;
				texfilename = namepair.second;
			}
			else
			{
				texfilename = namepair.first;
				if (helper::GetExtension(namepair.second) == "sph")
					sphfileName = namepair.second;
				if (helper::GetExtension(namepair.second) == "spa")
					spafileName = namepair.second;
			}
		}
		else
		{
			if (helper::GetExtension(pmdmaterials[i].texfilePath) == "sph")
			{
				sphfileName = pmdmaterials[i].texfilePath;
				texfilename = "";
			}
			else if (helper::GetExtension(pmdmaterials[i].texfilePath) == "spa")
			{
				spafileName = pmdmaterials[i].texfilePath;
				texfilename = "";
			}
			else
			{
				texfilename = pmdmaterials[i].texfilePath;
			}
		}

		//モデルとテクスチャパスからアプリケーションからのテクスチャパスを得る
		if (!texfilename.empty())
		{
			auto texfilePath = helper::GetTexturePathFromModelAndTexPath(strModelPath, texfilename.c_str());
			m_textureResources[i] = m_dx12.GetTextureByPath(texfilePath.c_str());
		}
		if (!sphfileName.empty())
		{
			auto sphfilePath = helper::GetTexturePathFromModelAndTexPath(strModelPath, sphfileName.c_str());
			m_sphResources[i] = m_dx12.GetTextureByPath(sphfilePath.c_str());
		}
		if (!spafileName.empty())
		{
			auto spafilePath = helper::GetTexturePathFromModelAndTexPath(strModelPath, spafileName.c_str());
			m_spaResources[i] = m_dx12.GetTextureByPath(spafilePath.c_str());
		}

	}


	unsigned short bonenum = 0;
	fread(&bonenum, sizeof(bonenum), 1, fp);

#pragma pack(1)
	struct PMDBone
	{
		char bonename[20];
		unsigned short parentNo;
		unsigned short nextNo;
		unsigned char type;
		unsigned short ikBoneNo;
		XMFLOAT3 pos;
	};
#pragma pack()

	std::vector<PMDBone> pmdbones(bonenum);
	fread(pmdbones.data(), sizeof(PMDBone), bonenum, fp);

	m_boneNameArray.resize(pmdbones.size());
	m_boneNodeAddressArray.resize(pmdbones.size());
	m_kneeIdx.clear();
	//ボーンノードマップ作製
	for (int i = 0; i < pmdbones.size(); ++i)
	{
		auto& pb = pmdbones[i];
		auto& node = m_boneNodetable[pb.bonename];
		node.boneidx = i;
		node.startPos = pb.pos;
		node.bonetype = pb.type;
		node.parentBone = pb.parentNo;
		node.ikparentBone = pb.ikBoneNo;

		m_boneNameArray[i] = pb.bonename;
		m_boneNodeAddressArray[i] = &node;

		std::string bonename = pb.bonename;
		if (bonename.find("ひざ") != std::string::npos)
		{
			m_kneeIdx.emplace_back(i);
		}
	}

	//親子関係を構築
	for (auto& pb : pmdbones)
	{
		//親インデックスをチェック(ありえない番号ならとばす)
		if (pb.parentNo >= pmdbones.size())
			continue;

		auto parentname = m_boneNameArray[pb.parentNo];
		m_boneNodetable[parentname].children.emplace_back(&m_boneNodetable[pb.bonename]);
	}

	m_boneMatAndQuat.resize(pmdbones.size());

	//ボーンを初期化

	std::fill(m_boneMatAndQuat.begin(), m_boneMatAndQuat.end(), m_MatAndQuatsIdentity);
	XMFLOAT4 f4(3, 2, 1, 1);
	XMVECTOR a(XMLoadFloat4(&f4));
	auto mqt = XMMatrixRotationQuaternion(a);
	uint16_t iknum = 0;
	fread(&iknum, sizeof(iknum), 1, fp);

	m_ikdata.resize(iknum);

	for (auto& ik : m_ikdata)
	{
		fread(&ik.boneIdx, sizeof(ik.boneIdx), 1, fp);
		fread(&ik.targetIdx, sizeof(ik.targetIdx), 1, fp);
		uint8_t chainlen = 0;
		fread(&chainlen, sizeof(chainlen), 1, fp);
		ik.nodeIdxes.resize(chainlen);
		fread(&ik.iterations, sizeof(ik.iterations), 1, fp);
		fread(&ik.limit, sizeof(ik.limit), 1, fp);
		if (chainlen == 0)
			continue;//間ノード数が0ならばここで終わり
		fread(ik.nodeIdxes.data(), sizeof(ik.nodeIdxes[0]), chainlen, fp);
	}

	std::reverse(m_ikdata.begin(), m_ikdata.end());
#ifdef _DEBUG
	auto getnameframeidx = [&](uint16_t idx)->std::string
	{
		auto it = find_if(m_boneNodetable.begin(), m_boneNodetable.end(),
			[idx](const std::pair<std::string, BoneNode>& obj)
			{
				return obj.second.boneidx == idx;
			});

		if (it != m_boneNodetable.end())
		{
			return it->first;
		}
		else
		{
			return "";
		}
	};

	for (auto& ik : m_ikdata)
	{
		std::ostringstream oss;
		oss << "IKボーン番号 = " << ik.boneIdx << ":" << getnameframeidx(ik.boneIdx) << std::endl;

		for (auto& node : ik.nodeIdxes)
		{
			oss << "\t ノードボーン = " << node << ":" << getnameframeidx(node) << std::endl;
		}

		OutputDebugString(oss.str().c_str());
	}

#endif // _DEBUG
	


	//表情リスト
	uint16_t skinnum = 0;
	fread(&skinnum, sizeof(skinnum), 1, fp);
#pragma pack(1)
	struct PMDskin
	{
		char skinname[20];
		uint8_t type;
		std::vector<skindata> vertexdata;
	};
#pragma pack()

	std::vector<PMDskin> skin(skinnum);

	for (auto& sk : skin)
	{
		fread(&sk.skinname, sizeof(sk.skinname), 1, fp);
		DWORD skinvercnt = 0;
		fread(&skinvercnt, sizeof(skinvercnt), 1, fp);
		sk.vertexdata.resize(skinvercnt);
		m_morphsData[sk.skinname].resize(skinvercnt);
		fread(&sk.type, sizeof(sk.type), 1, fp);
		fread(m_morphsData[sk.skinname].data(), sizeof(m_morphsData[sk.skinname][0]), skinvercnt, fp);

	}
	uint8_t skindispcnt = 0;
	fread(&skindispcnt, sizeof(skindispcnt), 1, fp);
	std::map<std::string,uint16_t> skinIDX;
	for (int i = 0, j = 0; i < skindispcnt; ++i)
	{
		if (skin[i].type == 0)
			++j;

		fread(&skinIDX[skin[i + j].skinname], sizeof(skinIDX[skin[i + j].skinname]), 1, fp);
	}
	uint8_t bonedispnamecnt = 0;
	fread(&bonedispnamecnt, sizeof(bonedispnamecnt), 1, fp);

	std::vector<char[50]> dispname(bonedispnamecnt);
	fread(dispname.data(), sizeof(dispname[0]), bonedispnamecnt , fp);

	DWORD bonedispcnt = 0;
	fread(&bonedispcnt, sizeof(bonedispcnt), 1, fp);

	struct BoneDisp
	{
		uint8_t boneIDX;
		uint8_t FfIDX[2];
	};

	std::vector<BoneDisp> bonedisp(bonedispcnt);
	fread(bonedisp.data(), sizeof(bonedisp[0]), bonedispcnt, fp);

	BYTE engname_comp = 0;
	fread(&engname_comp, sizeof(engname_comp), 1, fp);

	if (engname_comp == 1)
	{
		char modelname_eng[20] = {};
		char comment_eng[256] = {};
		fread(modelname_eng, sizeof(modelname_eng), 1, fp);
		fread(comment_eng, sizeof(comment_eng), 1, fp);

		std::vector<char[20]> bonename_eng(bonenum);
		fread(bonename_eng.data(), sizeof(bonename_eng[0]), bonenum, fp);

		std::vector<char[20]> skinname_eng(skinnum - 1);
		fread(skinname_eng.data(), sizeof(skinname_eng[0]), skinnum - 1, fp);

		std::vector<char[50]> bonedispname_eng(bonedispnamecnt);
		fread(bonedispname_eng.data(), sizeof(bonedispname_eng[0]), bonedispnamecnt, fp);
	}
	//std::vector<char> nameeng(bonedispcnt);
	//fread(nameeng.data(), sizeof(nameeng[0]), bonedispcnt, fp);
	std::vector<char[100]>toontexlist(10);
	fread(toontexlist.data(), sizeof(toontexlist[0]), 10, fp);
	for (int i = 0; i < (int)pmdmaterials.size(); ++i)
	{
		if (!m_toonResources[i])
		{
			std::string toonFilePath = {};
			std::string defaltetoonFilePath = {"asset/toon/"};
			if (pmdmaterials[i].toonIdx > toontexlist.size())
			{
				continue;
			}
			char number[20];
			sprintf(number, "toon%02d.bmp", pmdmaterials[i].toonIdx + 1);
			if (strcmp(toontexlist[pmdmaterials[i].toonIdx], number) == 0)
			{
				toonFilePath = defaltetoonFilePath + number;
			}
			else
			{
				 toonFilePath = helper::GetTexturePathFromModelAndTexPath(strModelPath, (toontexlist[pmdmaterials[i].toonIdx]));
			}
			m_toonResources[i] = m_dx12.GetTextureByPath(toonFilePath.c_str());
		}
	}
	DWORD rigitbodyNum = 0;
	fread(&rigitbodyNum, sizeof(rigitbodyNum), 1, fp);
	m_rigiddata.resize(rigitbodyNum);
	for (int i = 0; i < rigitbodyNum; i++)
	{
		fread(&m_rigiddata[i].name, sizeof(m_rigiddata[i].name), 1, fp);
		fread(&m_rigiddata[i].relboneIdx, sizeof(m_rigiddata[i].relboneIdx), 1, fp);
		fread(&m_rigiddata[i].groupIdx, sizeof(m_rigiddata[i].groupIdx), 1, fp);
		fread(&m_rigiddata[i].grouptarget, sizeof(m_rigiddata[i].grouptarget), 1, fp);
		fread(&m_rigiddata[i].shape_type, sizeof(BYTE), 1, fp);
		fread(&m_rigiddata[i].w, sizeof(m_rigiddata[i].w), 1, fp);
		fread(&m_rigiddata[i].h, sizeof(m_rigiddata[i].h), 1, fp);
		fread(&m_rigiddata[i].d, sizeof(m_rigiddata[i].d), 1, fp);
		fread(&m_rigiddata[i].pos, sizeof(m_rigiddata[i].pos), 1, fp);
		fread(&m_rigiddata[i].rot, sizeof(m_rigiddata[i].rot), 1, fp);
		fread(&m_rigiddata[i].weight, sizeof(m_rigiddata[i].weight), 1, fp);
		fread(&m_rigiddata[i].posdim, sizeof(m_rigiddata[i].posdim), 1, fp);
		fread(&m_rigiddata[i].rotdim, sizeof(m_rigiddata[i].rotdim), 1, fp);
		fread(&m_rigiddata[i].recoil, sizeof(m_rigiddata[i].recoil), 1, fp);
		fread(&m_rigiddata[i].friction, sizeof(m_rigiddata[i].friction), 1, fp);
		fread(&m_rigiddata[i].rigid_type, sizeof(BYTE), 1, fp);
		m_rigiddata[i].grouptarget = 0xFFFF - m_rigiddata[i].grouptarget;
	}
	

	DWORD jointNum = 0;
	fread(&jointNum, sizeof(jointNum), 1, fp);

	m_joint.resize(jointNum);

	for (int i = 0; i < jointNum; i++)
	{
		fread(&m_joint[i].name, sizeof(m_joint[i].name), 1, fp);
		fread(&m_joint[i].rigit_a, sizeof(m_joint[i].rigit_a), 1, fp);
		fread(&m_joint[i].rigit_b, sizeof(m_joint[i].rigit_b), 1, fp);
		fread(&m_joint[i].pos, sizeof(m_joint[i].pos), 1, fp);
		fread(&m_joint[i].rot, sizeof(m_joint[i].rot), 1, fp);
		fread(&m_joint[i].constrain_pos1, sizeof(m_joint[i].constrain_pos1), 1, fp);
		fread(&m_joint[i].constrain_pos2, sizeof(m_joint[i].constrain_pos2), 1, fp);
		fread(&m_joint[i].constrain_rot1, sizeof(m_joint[i].constrain_rot1), 1, fp);
		fread(&m_joint[i].constrain_rot2, sizeof(m_joint[i].constrain_rot2), 1, fp);
		fread(&m_joint[i].spring_pos, sizeof(m_joint[i].spring_pos), 1, fp);
		fread(&m_joint[i].spring_rot, sizeof(m_joint[i].spring_rot), 1, fp);
	}

	fclose(fp);

	return S_OK;
}

//誤差の範囲内かどうかに使用する定数
constexpr float epsilon = 0.0005f;

void PMDmodel::SolveCCDIK(const PMDIK& ik)
{
	//ターゲット
	auto tagetbonenode = m_boneNodeAddressArray[ik.boneIdx];
	auto tagetoriginpos = XMLoadFloat3(&tagetbonenode->startPos);

	const auto& parentmat = m_boneMatAndQuat[m_boneNodeAddressArray[ik.targetIdx]->boneidx].boneMatrieces;
	XMVECTOR det = {};
	auto invparentmat = XMMatrixInverse(&det, parentmat);
	auto targetnextpos = XMVector3Transform(tagetoriginpos, m_boneMatAndQuat[ik.boneIdx].boneMatrieces * invparentmat);

	//IKの間にあるボーンの座標を入れておく(逆順注意)
	std::vector<XMVECTOR> bonepositions;
	//末端ノード
	auto endpos = XMLoadFloat3(&m_boneNodeAddressArray[ik.targetIdx]->startPos);
	//endpos = XMVector3Transform(
	//XMLoadFloat3(&m_boneNodeAddressArray[ik.targetIdx]->startPos),
	////m_boneMatAndQuat[ik.targetIdx]);
	//XMMatrixIdentity());


	//中間ノード(ルートを含む)
	for (auto& cidx : ik.nodeIdxes)
	{
		bonepositions.push_back(XMLoadFloat3(&m_boneNodeAddressArray[cidx]->startPos));
		//bonepositions.emplace_back(XMVector3Transform(XMLoadFloat3(&m_boneNodeAddressArray[cidx]->startPos),
		//m_boneMatAndQuat[cidx] ));
	}

	std::vector<XMMATRIX> mats(bonepositions.size());
	fill(mats.begin(), mats.end(), XMMatrixIdentity());

	auto iklimit = ik.limit * XM_PI;
	//IKに設定されている試行回数だけ繰り返す
	for (int c = 0; c < ik.iterations; ++c)
	{
		//if (XMVector3Length(XMVectorSubtract(endpos, targetnextpos)).m128_f32[0] <= epsilon)
		//	continue;

		//それぞれのボーンを遡りながら角度制限に引っ掛からないように曲げていく
		for (int bidx = 0; bidx < bonepositions.size(); ++bidx)
		{
			const auto& pos = bonepositions[bidx];

			//現在のノードから末端までと、現在のノードからターゲットまでのベクトルを作る
			auto vectoend = XMVectorSubtract(endpos, pos);
			auto vectotarget = XMVectorSubtract(targetnextpos, pos);
			vectoend = XMVector3Normalize(vectoend);
			vectotarget = XMVector3Normalize(vectotarget);

			//ほぼ同じベクトルになってしまった場合は外積できないため次のボーンに引き渡す
			if (XMVector3Length(XMVectorSubtract(vectoend, vectotarget)).m128_f32[0] <= epsilon)
				continue;

			//外積計算及び角度計算
			auto cross = XMVector3Normalize(XMVector3Cross(vectoend, vectotarget));
			float angle = XMVector3AngleBetweenVectors(vectoend, vectotarget).m128_f32[0];
			angle = min(angle, iklimit);						//回転限界補正
			XMMATRIX rot = XMMatrixRotationAxis(cross, angle);	//回転行列
			//posを中心に回転
			auto mat = XMMatrixTranslationFromVector(-pos) * rot * XMMatrixTranslationFromVector(pos);

			mats[bidx] *= mat;		//回転行列を保持しておく
			//対象となる点をすべて回転させる
			for (auto idx = bidx - 1; idx >= 0; --idx)
			{
				//自分を回転させる必要はない
				bonepositions[idx] = XMVector3Transform(bonepositions[idx], mat);
			}
			endpos = XMVector3Transform(endpos, mat);
			//もし政界に近くなってたらループを抜ける
			if (XMVector3Length(XMVectorSubtract(endpos, targetnextpos)).m128_f32[0] <= epsilon)
				break;
		}
	}

	int idx = 0;
	for (auto& cidx : ik.nodeIdxes)
	{
		m_boneMatAndQuat[cidx].boneMatrieces = mats[idx];
		++idx;
	}
	auto node = m_boneNodeAddressArray[ik.nodeIdxes.back()];
	RecursiveMatrixMultiply(node, parentmat, true);
}

void PMDmodel::SolveCosineIK(const PMDIK& ik)
{
	std::vector<XMVECTOR> positions;	//IK構成点を保存
	std::array<float, 2> edgelens;		//IKそれぞれのボーン間の距離を保存

	//ターゲット（末端ボーンではなく、末端ボーンが近づく目標ボーンの座標を取得）
	auto& tagetnode = m_boneNodeAddressArray[ik.boneIdx];
	auto targetpos = XMVector3Transform(XMLoadFloat3(&tagetnode->startPos), m_boneMatAndQuat[ik.boneIdx].boneMatrieces);

	//IKチェーンが逆順なので逆に並ぶように
	//末端ボーン
	auto endnode = m_boneNodeAddressArray[ik.targetIdx];
	positions.emplace_back(XMLoadFloat3(&endnode->startPos));

	//中間及びルートボーン
	for (auto& chainboneidx : ik.nodeIdxes)
	{
		auto bonenode = m_boneNodeAddressArray[chainboneidx];
		positions.emplace_back(XMLoadFloat3(&bonenode->startPos));
	}

	//分かりずらいので逆にする
	reverse(positions.begin(), positions.end());

	//元の長さを測る
	edgelens[0] = XMVector3Length(XMVectorSubtract(positions[1], positions[0])).m128_f32[0];
	edgelens[1] = XMVector3Length(XMVectorSubtract(positions[2], positions[1])).m128_f32[0];

	//ルートボーン座標変換(逆順になっているため使用するインデックスに注意)
	positions[0] = XMVector3Transform(positions[0], m_boneMatAndQuat[ik.nodeIdxes[1]].boneMatrieces);
	//先端ボーン
	positions[2] = XMVector3Transform(positions[2], m_boneMatAndQuat[ik.boneIdx].boneMatrieces);

	//ルートから端末へのベクトルを作る
	auto linearvec = XMVectorSubtract(positions[2], positions[0]);

	float A = XMVector3Length(linearvec).m128_f32[0];
	float B = edgelens[0];
	float C = edgelens[1];

	linearvec = XMVector3Normalize(linearvec);

	//ルートから真ん中への角度計算
	float theta1 = acosf((A * A + B * B - C * C) / (2 * A * B));
	if (isnan(theta1))
	{
		theta1 = acosf(1);
	}

	//真ん中からターゲットへの角度計算
	float theta2 = acosf((B * B + C * C - A * A) / (2 * B * C));
	if (isnan(theta2))
	{
		theta2 = acosf(-1);
	}

	//「軸」を求める
	//もし真ん中が「ひざ」であった場合には強制的にX軸とする。
	XMVECTOR axis = {};
	if (find(m_kneeIdx.begin(), m_kneeIdx.end(), ik.nodeIdxes[0]) == m_kneeIdx.end())
	{
		auto vm = XMVector3Normalize(XMVectorSubtract(positions[2], positions[0]));
		auto vt = XMVector3Normalize(XMVectorSubtract(targetpos, positions[0]));
		axis = XMVector3Cross(vt, vm);
	}
	else
	{
		auto right = XMFLOAT3(1, 0, 0);
		axis = XMLoadFloat3(&right);
	}

	auto parent = m_boneNodeAddressArray[ik.boneIdx]->parentBone;

	auto a = m_boneNameArray[ik.boneIdx];
	auto b = m_boneNameArray[ik.targetIdx];
	auto c = m_boneNameArray[ik.nodeIdxes[0]];
	auto d = m_boneNameArray[ik.nodeIdxes[1]];


	//「注意」IKチェーンは根っこに向かってから数えられるため1が根っこに近い
	auto mat1 = XMMatrixTranslationFromVector(-positions[0]);
	mat1 *= XMMatrixRotationAxis(axis, theta1);
	mat1 *= XMMatrixTranslationFromVector(positions[0]);

	auto mat2 = XMMatrixTranslationFromVector(-positions[1]);
	mat2 *= XMMatrixRotationAxis(axis, theta2 - XM_PI);
	mat2 *= XMMatrixTranslationFromVector(positions[1]);


	m_boneMatAndQuat[ik.nodeIdxes[1]].boneMatrieces *= mat1;
	m_boneMatAndQuat[ik.nodeIdxes[0]].boneMatrieces = mat2 * m_boneMatAndQuat[ik.nodeIdxes[1]].boneMatrieces;
	m_boneMatAndQuat[ik.targetIdx] = m_boneMatAndQuat[ik.nodeIdxes[0]] ;
	
}

void PMDmodel::SolveLookAt(const PMDIK& ik)
{
	auto parent = m_boneNodeAddressArray[ik.boneIdx]->parentBone;
	auto rootnode = m_boneNodeAddressArray[ik.nodeIdxes[0]];
	auto tagetnode = m_boneNodeAddressArray[ik.targetIdx];

	auto opos1 = XMLoadFloat3(&rootnode->startPos);
	auto tpos1 = XMLoadFloat3(&tagetnode->startPos);

	auto opos2 = XMVector3Transform(opos1, m_boneMatAndQuat[ik.nodeIdxes[0]].boneMatrieces);
	auto tpos2 = XMVector3Transform(tpos1, m_boneMatAndQuat[ik.boneIdx].boneMatrieces);

	auto originvec = XMVectorSubtract(tpos1, opos1);
	auto targetvec = XMVectorSubtract(tpos2, opos2);

	originvec = XMVector3Normalize(originvec);
	targetvec = XMVector3Normalize(targetvec);

	XMMATRIX mat = XMMatrixTranslationFromVector(-opos2) *
		LookAtMatrix(originvec, targetvec, XMFLOAT3(0, 1, 0), XMFLOAT3(1, 0, 0)) *
		XMMatrixTranslationFromVector(opos2);

	auto a = m_boneNameArray[ik.boneIdx];
	auto b = m_boneNameArray[ik.targetIdx];
	auto c = m_boneNameArray[ik.nodeIdxes[0]];
	auto d = m_boneNameArray[parent];

	XMVECTOR tarnsnode;
	XMVECTOR rotnode;
	XMVECTOR scalnode;
	XMVECTOR rotparent;

	XMMatrixDecompose(&scalnode, &rotparent, &tarnsnode, m_boneMatAndQuat[parent].boneMatrieces);
	XMMatrixDecompose(&scalnode, &rotnode, &tarnsnode, m_boneMatAndQuat[ik.nodeIdxes[0]].boneMatrieces);

	//m_boneMatAndQuat[ik.nodeIdxes[0]] = XMMatrixAffineTransformation(scalnode, rotnode, rotparent, tarnsnode) * mat;
	//m_boneMatAndQuat[ik.targetIdx] = m_boneMatAndQuat[parent];
	m_boneMatAndQuat[ik.nodeIdxes[0]].boneMatrieces = ( m_boneMatAndQuat[ik.boneIdx].boneMatrieces * m_boneMatAndQuat[parent].boneMatrieces);
	m_boneMatAndQuat[ik.targetIdx].boneMatrieces = m_boneMatAndQuat[parent].boneMatrieces;
	m_boneMatAndQuat[parent].boneMatrieces = m_boneMatAndQuat[ik.nodeIdxes[0]].boneMatrieces;

}

void PMDmodel::IKSolve(int frameNo)
{
	auto enabledata = m_motion->GetIKEnableData();
	auto it = find_if(enabledata->rbegin(), enabledata->rend(),
		[frameNo](const VMDmotion::VMDIKEnable& ikenble)
		{
			return ikenble.frameNo <= frameNo;
		});


	for (auto& ik : m_ikdata)
	{
		if (it != enabledata->rend())
		{
			auto ikenabelit = it->ikEnableTable.find(m_boneNameArray[ik.boneIdx]);
			if (ikenabelit != it->ikEnableTable.end())
			{
				if (!ikenabelit->second)
					continue;
			}
		}
		auto childrennodescount = ik.nodeIdxes.size();

		switch (childrennodescount)
		{
		case 0:
			assert(0);
			continue;
		case 1://間のボーン数が1のときはLookAt
			SolveLookAt(ik);
			break;
		case 2://間のボーン数が2のときは余弦定理IK
			SolveCosineIK(ik);
			break;
		default://3以上の時はCCD-IK
			SolveCCDIK(ik);
		}
	}
}


