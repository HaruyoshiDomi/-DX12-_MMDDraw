#include "main.h"
#include "Render.h"
#include "VMDmotion.h"
#include "XMFLOAT_Helper.h"
#include "PMXmodel.h"
#include <fstream>
#include <array>

PMXmodel::PMXmodel(const char* filepath, Render& dx12) : Model(dx12)
{
	auto result = CreateRootSignature();
	result = CreateGraphicsPipeline();
	m_transform.world = XMMatrixIdentity();
	result = LoadPMXFile(filepath);
	result = CreateTransformView();
	result = CreateMaterialData();
	result = CreateMaterialAndTextureView();

}

PMXmodel::~PMXmodel()
{
}

void PMXmodel::Update(XMMATRIX& martrix)
{
	m_mappeboneMatAndQuats[0].boneMatrieces = martrix;
	if (m_motion)
	{
		if (m_motion->GetMotionFlag())
		{
			m_motion->UpdateMotion();
			RecursiveMatrixMultiply(&m_boneNodetable["センター"], XMMatrixIdentity());
		}
		std::copy(m_boneMatAndQuat.begin(), m_boneMatAndQuat.end(), m_mappeboneMatAndQuats + 1);
		this->UpdateModelMorf();
	}

}

void PMXmodel::Draw()
{

	m_dx12.CommandList()->SetPipelineState(this->m_pipeline.Get());
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

	auto cbvsrvIncSize = m_dx12.Device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 6;
	for (auto& m : m_materials)
	{
		m_dx12.CommandList()->SetGraphicsRootDescriptorTable(2, materialH);
		m_dx12.CommandList()->DrawIndexedInstanced(m.indicesNum , 2, idxOffset, 0, 0);
		materialH.ptr += cbvsrvIncSize;
		idxOffset += m.indicesNum ;
	}
}

HRESULT PMXmodel::CreateRootSignature()
{
	//レンジ
	CD3DX12_DESCRIPTOR_RANGE  descTblRanges[4] = {};				//テクスチャと定数の２つ
	descTblRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);	//定数[b0](ビュープロジェクション用)
	descTblRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);	//定数[b1](ワールド、ボーン用)
	descTblRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);	//定数[b2](マテリアル用)
	descTblRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0);	//テクスチャ４つ(基本とsphとspaとトゥーン)

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

HRESULT PMXmodel::CreateGraphicsPipeline()
{
	if (!m_VSBlob && !m_PSBlob)
	{
		auto result = CreateVS(L"PMXVS.cso");
		if (FAILED(result))
		{
			assert(0);
			return S_FALSE;
		}

		result = CreatePS(L"PMXPS.cso");
		if (FAILED(result))
		{
			assert(0);
			return S_FALSE;
		}
	}
	WORD a;
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "EXUVI",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "EXUVII",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "EXUVIII",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "EXUVIV",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TYPE",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "BONENO",0,DXGI_FORMAT_R32G32B32A32_SINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "WEIGHT",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "C",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "RZ",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "RO",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 }
		//{ "EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_dx12.CommandList()->OMSetBlendFactor(blendFactor);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = m_rootSignature.Get();
	gpipeline.VS = CD3DX12_SHADER_BYTECODE(m_VSBlob.Get());
	gpipeline.PS = CD3DX12_SHADER_BYTECODE(m_PSBlob.Get());

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//中身は0xffffffff

	gpipeline.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;
	for (int i = 0; i < _countof(gpipeline.BlendState.RenderTarget); ++i)
	{
		//見やすくするため変数化
		auto& rt = gpipeline.BlendState.RenderTarget[i];
		rt.BlendEnable = TRUE;
		rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		rt.BlendOp = D3D12_BLEND_OP_ADD;
		rt.SrcBlendAlpha = D3D12_BLEND_ONE;
		rt.DestBlendAlpha = D3D12_BLEND_ZERO;
		rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	gpipeline.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;					//カリングバック

	gpipeline.DepthStencilState.DepthEnable = true;								//深度バッファを使う
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	//全て書き込み
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;			//小さい方を採用
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpipeline.DepthStencilState.StencilEnable = false;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;						//レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements = _countof(inputLayout);					//レイアウト配列数

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;	//ストリップ時のカットなし
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//三角形で構成

	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;						//0～1に正規化されたRGBA

	gpipeline.SampleDesc.Count = 1;												//サンプリングは1ピクセルにつき１
	gpipeline.SampleDesc.Quality = 0;											//クオリティは最低
	auto result = m_dx12.Device()->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(m_pipeline.ReleaseAndGetAddressOf()));
	if (FAILED(result))
	{
		assert(SUCCEEDED(result));
	}
	return result;
}

HRESULT PMXmodel::CreateMaterialData()
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

HRESULT PMXmodel::CreateMaterialAndTextureView()
{
	D3D12_DESCRIPTOR_HEAP_DESC materialDescHeapDesc = {};
	materialDescHeapDesc.NumDescriptors = m_materials.size() * 6;//マテリアル数ぶん(定数1つ、テクスチャ3つ)
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

		if (m_subtextureResources[i])
		{
			srvDesc.Format = m_subtextureResources[i]->GetDesc().Format;
			m_dx12.Device()->CreateShaderResourceView(m_subtextureResources[i].Get(), &srvDesc, matDescHeapH);
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

HRESULT PMXmodel::CreateTransformView()
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

HRESULT PMXmodel::LoadPMXFile(const char* filepath)
{
	struct PMXHeader
	{
		char signature[4];
		float vertion;
		uint8_t bytesize;
	};

	FILE* fp;

	std::string strModelPath = filepath;

	std::array<BYTE, 4> pmxHeader{};
	constexpr std::array<BYTE, 4> PMX_MAGIC_NUMBER{ 0x50, 0x4d, 0x58, 0x20 };
	enum HeaderDataIndex
	{
		ENCODING_FORMAT,
		NUMBER_OF_ADD_UV,
		VERTEX_INDEX_SIZE,
		TEXTURE_INDEX_SIZE,
		MATERIAL_INDEX_SIZE,
		BONE_INDEX_SIZE,
		MORF_INDEX_SIZE,
		RIGID_BODY_INDEX_SIZE
	};

	auto err = fopen_s(&fp, strModelPath.c_str(), "rb");
	if (!fp)
	{
		assert(0);
		fclose(fp);
		return ERROR_FILE_NOT_FOUND;
	}

	for (int i = 0; i < 4; i++)
	{
		fread(&pmxHeader[i], sizeof(BYTE), 1, fp);
	}

	if (pmxHeader != PMX_MAGIC_NUMBER)
	{
		assert(0);
		fclose(fp);
		return ERROR_FILE_NOT_FOUND;
	}

	float vertion = 0.0f;
	fread(&vertion, sizeof(vertion), 1, fp);
	if (!XMScalarNearEqual(vertion, 2.0f, g_XMEpsilon.f[0]))
	{
		assert(0);
		fclose(fp);
		return ERROR_FILE_NOT_FOUND;
	}
	BYTE pmxdatalen = 0;
	fread(&pmxdatalen, sizeof(pmxdatalen), 1, fp);

	std::array<BYTE, 8> hederData{};

	for (int i = 0; i < pmxdatalen; i++)
	{
		fread(&hederData[i], sizeof(BYTE), 1, fp);
	}
	std::vector<std::wstring> name(4);
	unsigned arreylen = 0;
	for (int i = 0; i < 4; i++)
	{
		getPMXStringUTF16(fp, name[i]);
		std::cout << helper::GetStringFromWideString(name[i]).c_str() << std::endl;
	}

	auto testnameutf8 = helper::GetUTF8FromWideString(name[0]);
	m_name = testnameutf8;
	std::cout << testnameutf8.c_str() << std::endl;
	uint32_t numVer = 0;

	fread(&numVer, sizeof(numVer), 1, fp);

	enum Type
	{
		BDEF1,
		BDEF2,
		BDEF4,
		SDEF,
	};

	m_vertice.resize(numVer);
	m_originalVertices.resize(numVer);
	// ボーンウェイト

	uint16_t boneindes[4] = {};

	for (int i = 0; i < numVer; i++)
	{
		fread(&m_vertice[i].pos, sizeof(XMFLOAT3), 1, fp);
		fread(&m_vertice[i].normal, sizeof(XMFLOAT3), 1, fp);
		fread(&m_vertice[i].uv, sizeof(XMFLOAT2), 1, fp);
		m_originalVertices[i].pos = m_vertice[i].pos;
		m_originalVertices[i].uv = m_vertice[i].uv;
		if (hederData[NUMBER_OF_ADD_UV] != 0)
		{
			for (int j = 0; j < hederData[NUMBER_OF_ADD_UV]; ++j)
			{
				fread(&m_vertice[i].additionaluv[j], sizeof(XMFLOAT4), 1, fp);
			}
		}
		uint8_t weightMethot = 0;
		fread(&weightMethot, sizeof(weightMethot), 1, fp);
		switch (weightMethot)
		{
		case Type::BDEF1:
			m_vertice[i].type = Type::BDEF1;
			fread(&boneindes[0], hederData[BONE_INDEX_SIZE], 1, fp);
			m_vertice[i].born[0] = boneindes[0];
			m_vertice[i].born[1] = -1;
			m_vertice[i].born[2] = -1;
			m_vertice[i].born[3] = -1;
			m_vertice[i].weight.x = 1.0f;
			break;
		case Type::BDEF2:
			m_vertice[i].type = Type::BDEF2;
			fread(&boneindes[0], hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&boneindes[1], hederData[BONE_INDEX_SIZE], 1, fp);
			m_vertice[i].born[0] = boneindes[0];
			m_vertice[i].born[1] = boneindes[1];
			m_vertice[i].born[2] = -1;
			m_vertice[i].born[3] = -1;
			fread(&m_vertice[i].weight.x, sizeof(float), 1, fp);
			m_vertice[i].weight.y = 1.0f - m_vertice[i].weight.x;
			break;
		case Type::BDEF4:
			m_vertice[i].type = Type::BDEF4;
			fread(&boneindes[0], hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&boneindes[1], hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&boneindes[2], hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&boneindes[3], hederData[BONE_INDEX_SIZE], 1, fp);
			m_vertice[i].born[0] = boneindes[0];
			m_vertice[i].born[1] = boneindes[1];
			m_vertice[i].born[2] = boneindes[2];
			m_vertice[i].born[3] = boneindes[3];
			fread(&m_vertice[i].weight.x, sizeof(float), 1, fp);
			fread(&m_vertice[i].weight.y, sizeof(float), 1, fp);
			fread(&m_vertice[i].weight.z, sizeof(float), 1, fp);
			fread(&m_vertice[i].weight.w, sizeof(float), 1, fp);
			break;
		case Type::SDEF:
			m_vertice[i].type = Type::SDEF;
			fread(&boneindes[0], hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&boneindes[1], hederData[BONE_INDEX_SIZE], 1, fp);
			m_vertice[i].born[0] = boneindes[0];
			m_vertice[i].born[1] = boneindes[1];
			m_vertice[i].born[2] = -1;
			m_vertice[i].born[3] = -1;
			fread(&m_vertice[i].weight.x, sizeof(float), 1, fp);
			m_vertice[i].weight.y = 1.0f - m_vertice[i].weight.x;
			fread(&m_vertice[i].c, sizeof(XMFLOAT3), 1, fp);
			fread(&m_vertice[i].r0, sizeof(XMFLOAT3), 1, fp);
			fread(&m_vertice[i].r1, sizeof(XMFLOAT3), 1, fp);
			break;
		}

		fread(&m_vertice[i].edgenif, sizeof(float), 1, fp);
	}

	//インデックス
	uint32_t indicesNum = 0;
	fread(&indicesNum, sizeof(indicesNum), 1, fp);
	std::vector<uint32_t> indices(indicesNum);
	for (int i = 0; i < indicesNum; ++i)
	{
		fread(&indices[i], hederData[VERTEX_INDEX_SIZE], 1, fp);
	}

	//テクスチャロード
	uint32_t Texnum = 0;
	fread(&Texnum, sizeof(Texnum), 1, fp);

	std::vector<wstring> texpath(Texnum);
	for (int i = 0; i < Texnum; ++i)
	{
		getPMXStringUTF16(fp, texpath[i]);
		if ((texpath[i].find(L"\\")) != wstring::npos)
		{
			auto p = helper::SplitFileName(texpath[i], '\\');
			texpath[i] = p.first + L'/' + p.second;
		}
		auto path = helper::GetWideStringFromString(filepath);
		texpath[i] = ChangeCNToJP(texpath[i]);
		texpath[i] = helper::GetTexturePathFromModelAndTexPath(path, texpath[i].c_str());
	}

	//マテリアル
	uint32_t materialnum = 0;
	fread(&materialnum, sizeof(materialnum), 1, fp);
	m_materials.resize(materialnum);
	m_textureResources.resize(materialnum);
	m_sphResources.resize(materialnum);
	m_spaResources.resize(materialnum);
	m_toonResources.resize(materialnum);
	m_subtextureResources.resize(materialnum);

	for (int i = 0; i < materialnum; i++)
	{
		//名前（日英）
		getPMXStringUTF16(fp, m_materials[i].name);
		getPMXStringUTF16(fp, m_materials[i].name_eng);
		fread(&m_materials[i].material.diffuse, sizeof(XMFLOAT4), 1, fp);
		fread(&m_materials[i].material.specular, sizeof(XMFLOAT3), 1, fp);
		fread(&m_materials[i].material.specularity, sizeof(float), 1, fp);
		fread(&m_materials[i].material.ambient, sizeof(XMFLOAT3), 1, fp);
		BYTE drawflag = 0;
		fread(&drawflag, sizeof(drawflag), 1, fp);
		if (drawflag & (1 << 0))
		{
			if (drawflag == 0x10)
			{
				m_materials[i].material.edgeFlg = true;
			}
		}
		fread(&m_materials[i].material.edgecolor, sizeof(XMFLOAT4), 1, fp);
		fread(&m_materials[i].material.edgesize, sizeof(float), 1, fp);
		fread(&m_materials[i].additional.texIdx, hederData[TEXTURE_INDEX_SIZE], 1, fp);
		fread(&m_materials[i].additional.spharIdx, hederData[TEXTURE_INDEX_SIZE], 1, fp);
		fread(&m_materials[i].additional.mode, sizeof(BYTE), 1, fp);
		fread(&m_materials[i].additional.toonflag, sizeof(BYTE), 1, fp);
		if (!m_materials[i].additional.toonflag)
		{
			fread(&m_materials[i].additional.toonIdx, hederData[TEXTURE_INDEX_SIZE], 1, fp);
		}
		else
		{
			std::string defaltetoonFilePath = { "asset/toon/" };
			int tnum = 0;
			char number[20];
			fread(&tnum, 1, 1, fp);
			sprintf(number, "toon%02d.bmp", tnum + 1);
			m_materials[i].additional.defaltetoontex = defaltetoonFilePath + number;
		}

		getPMXStringUTF16(fp, m_materials[i].additional.comment);
		fread(&m_materials[i].indicesNum, sizeof(uint32_t), 1, fp);

	}

	uint32_t bonenum = 0;

	struct PMXbone
	{
		std::string name;
		std::string name_eng;
		XMFLOAT3 pos;
		XMFLOAT3 coordofset;
		XMFLOAT3 fixedaxis;		//固定軸方向ベクトル
		XMFLOAT3 localaxisX;	//ローカルのX軸方向ベクトル
		XMFLOAT3 localaxisZ;	//ローカルのZ軸方向ベクトル
		int parentidx;			//親ボーン
		int transformlv;		//変形階層
		int childenidx;			//子ボーン（接続先）
		int imparttparentbone;	//付与親ボーンindex
		int externalparentkey;	//外部親変形値
		float impartrate;		//付与率
		int iktargetindex;
		int ikloopcount;
		float ikunitangle;		//ループ1回あたりの制限角度（ラジアン角）「PMDIK値の4倍」
		uint16_t flag;

		struct IKLink
		{
			int index;
			bool existangellimit;
			XMFLOAT3 limitangmin;
			XMFLOAT3 limitangmax;
		};
		std::vector<IKLink> ikLink;
	};

	fread(&bonenum, sizeof(bonenum), 1, fp);
	m_boneMatAndQuat.resize(bonenum);
	m_boneNameArray.resize(bonenum);
	m_boneNodeAddressArray.resize(bonenum);
	std::vector<PMXbone> bone(bonenum);
	//ボーンを初期化
	std::fill(m_boneMatAndQuat.begin(), m_boneMatAndQuat.end(), m_MatAndQuatsIdentity);

	//ボーン情報読み込み
	{
		int iklinksize = 0;
		uint8_t anglim = 0;
		enum BoneFlagMask
		{
			ACCESS_POINT = 0x0001,
			IK = 0x0020,
			IMPART_ROTATION = 0x0100,
			IMPART_TRANSLATION = 0x0200,
			AXIS_FIXING = 0x0400,
			LOCAL_AXIS = 0x0800,
			EXTERNAL_PARENT_TRANS = 0x2000,
		};

		for (int i = 0; i < bonenum; i++)
		{
			getPMXStringUTF16(fp, bone[i].name);
			getPMXStringUTF16(fp, bone[i].name_eng);
			fread(&bone[i].pos, sizeof(bone[i].pos), 1, fp);
			fread(&bone[i].parentidx, hederData[BONE_INDEX_SIZE], 1, fp);
			if (bone[i].parentidx >= bonenum)
			{
				bone[i].parentidx = -1;
			}
			fread(&bone[i].transformlv, sizeof(bone[i].transformlv), 1, fp);
			fread(&bone[i].flag, 2, 1, fp);

			if (bone[i].flag & ACCESS_POINT)
			{
				fread(&bone[i].childenidx, hederData[BONE_INDEX_SIZE], 1, fp);
				if (bone[i].childenidx >= bonenum)
				{
					bone[i].childenidx = -1;
				}
			}
			else
			{
				bone[i].childenidx = -1;
				fread(&bone[i].coordofset, sizeof(bone[i].coordofset), 1, fp);
			}

			if ((bone[i].flag & IMPART_ROTATION) || (bone[i].flag & IMPART_TRANSLATION))
			{
				fread(&bone[i].imparttparentbone, hederData[BONE_INDEX_SIZE], 1, fp);
				fread(&bone[i].impartrate, 4, 1, fp);
			}
			if (bone[i].flag & AXIS_FIXING)
			{
				fread(&bone[i].fixedaxis, sizeof(bone[i].fixedaxis), 1, fp);
			}
			if (bone[i].flag & LOCAL_AXIS)
			{
				fread(&bone[i].localaxisX, sizeof(bone[i].localaxisX), 1, fp);
				fread(&bone[i].localaxisZ, sizeof(bone[i].localaxisZ), 1, fp);
			}
			if (bone[i].flag & EXTERNAL_PARENT_TRANS)
			{
				fread(&bone[i].externalparentkey, 4, 1, fp);
			}
			if (bone[i].flag & IK)
			{
				fread(&bone[i].iktargetindex, hederData[BONE_INDEX_SIZE], 1, fp);
				fread(&bone[i].ikloopcount, 4, 1, fp);
				fread(&bone[i].ikunitangle, 4, 1, fp);
				fread(&iklinksize, 4, 1, fp);
				bone[i].ikLink.resize(iklinksize);
				for (int j = 0; j < iklinksize; j++)
				{
					fread(&bone[i].ikLink[j].index, hederData[BONE_INDEX_SIZE], 1, fp);
					fread(&anglim, sizeof(anglim), 1, fp);
					bone[i].ikLink[j].existangellimit = false;
					if (anglim == 1)
					{
						fread(&bone[i].ikLink[j].limitangmin, sizeof(XMFLOAT3), 1, fp);
						fread(&bone[i].ikLink[j].limitangmax, sizeof(XMFLOAT3), 1, fp);
					}
				}
			}
			else
			{
				bone[i].iktargetindex = -1;
			}
			//ボーンノードマップ作製
			auto& node = m_boneNodetable[bone[i].name];
			node.boneidx = i;
			node.startPos = bone[i].pos;
			node.parentBone = bone[i].parentidx;
			node.ikparentBone = bone[i].iktargetindex;

			m_boneNameArray[i] = bone[i].name;
			m_boneNodeAddressArray[i] = &node;
		}

		//親子関係構築
		for (auto& bone : bone)
		{
			if (bone.parentidx >= bonenum)
				continue;

			auto parentname = m_boneNameArray[bone.parentidx];
			m_boneNodetable[parentname].children.emplace_back(&m_boneNodetable[bone.name]);
		}

	}

	//モーフ（表情）
	int morfnum;
	fread(&morfnum, sizeof(morfnum), 1, fp);

	struct PMXmorfdata
	{
		std::string name;
		std::string name_eng;
		uint8_t panel;
		uint8_t type;
		int offsetnum;
	};
	std::vector<PMXmorfdata> morfdata(morfnum);
	for (int i = 0; i < morfnum; i++)
	{
		getPMXStringUTF16(fp, morfdata[i].name);
		getPMXStringUTF16(fp, morfdata[i].name_eng);
		fread(&morfdata[i].panel, 1, 1, fp);
		fread(&morfdata[i].type, 1, 1, fp);
		fread(&morfdata[i].offsetnum, 4, 1, fp);
		switch (morfdata[i].type)
		{
		case 0:
			break;
		case 1:
			m_morphsData[morfdata[i].name].resize(morfdata[i].offsetnum);
			for (int j = 0; j < morfdata[i].offsetnum; j++)
			{
				fread(&m_morphsData[morfdata[i].name][j].vertexIndex, hederData[VERTEX_INDEX_SIZE], 1, fp);
				fread(&m_morphsData[morfdata[i].name][j].posval, sizeof(XMFLOAT3), 1, fp);
				m_morphsData[morfdata[i].name][j].uv = { 0,0,0,0 };
			}
			
			break;
		case 2:
			break;
		case 3:
			m_morphsData[morfdata[i].name].resize(morfdata[i].offsetnum);
			for (int j = 0; j < morfdata[i].offsetnum; j++)
			{
				fread(&m_morphsData[morfdata[i].name][j].vertexIndex, hederData[VERTEX_INDEX_SIZE], 1, fp);
				fread(&m_morphsData[morfdata[i].name][j].uv, sizeof(XMFLOAT4), 1, fp);
				m_morphsData[morfdata[i].name][j].posval = { 0,0,0 };
			}
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;

		default:
			break;
		}

	}

	int flamenum;
	fread(&flamenum, sizeof(flamenum), 1, fp);


	std::string namemorf;
	std::string namemorfe;
	uint8_t flag;
	int numval;
	uint8_t typemb;
	int wwidx = 0;
	getPMXStringUTF16(fp, namemorf);
	getPMXStringUTF16(fp, namemorfe);
	fread(&flag, 1, 1, fp);
	fread(&numval, 4, 1, fp);
	fread(&typemb, 1, 1, fp);
	fread(&wwidx, hederData[BONE_INDEX_SIZE], 1, fp);
	getPMXStringUTF16(fp, namemorf);
	getPMXStringUTF16(fp, namemorfe);
	fread(&flag, 1, 1, fp);
	fread(&numval, 4, 1, fp);
	fread(&typemb, 1, 1, fp);
	fread(&wwidx, hederData[MORF_INDEX_SIZE], 1, fp);

	//＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝
	// 
	//バッファ書き込み

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_vertice.size() * sizeof(m_vertice[0]));

	auto result = m_dx12.Device()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_vb.ReleaseAndGetAddressOf()));

	//モーフ更新のためUnmapしない
	PMXVertice* vertMap = nullptr;
	result = m_vb->Map(0, nullptr, (void**)&vertMap);
	std::copy(m_vertice.begin(), m_vertice.end(), vertMap);

	m_vbView.BufferLocation = m_vb->GetGPUVirtualAddress();//バッファの仮想アドレス
	m_vbView.SizeInBytes = m_vertice.size() * sizeof(PMXVertice);//全バイト数
	m_vbView.StrideInBytes = sizeof(m_vertice[0]);//1頂点あたりのバイト数

	auto resDescBuf = CD3DX12_RESOURCE_DESC::Buffer(indices.size() * sizeof(indices[0]));


	result = m_dx12.Device()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDescBuf,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_ib.ReleaseAndGetAddressOf()));

	//作ったバッファにインデックスデータをコピー
	uint32_t* mappedIdx = nullptr;
	result = m_ib->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(indices.begin(), indices.end(), mappedIdx);
	m_ib->Unmap(0, nullptr);

	//インデックスバッファビューを作成
	m_ibView.BufferLocation = m_ib->GetGPUVirtualAddress();
	m_ibView.Format = DXGI_FORMAT_R32_UINT;
	m_ibView.SizeInBytes = indices.size() * sizeof(indices[0]);

	for (int i = 0; i < materialnum; i++)
	{
		//モデルとテクスチャパスからアプリケーションからのテクスチャパスを得る
		if (m_materials[i].additional.texIdx <= Texnum)
		{
			auto path = helper::GetStringFromWideString(texpath[m_materials[i].additional.texIdx]);
			m_textureResources[i] = m_dx12.GetTextureByPath(path.c_str());
		}

		if (m_materials[i].additional.mode == 1)
		{
			auto path = helper::GetStringFromWideString(texpath[m_materials[i].additional.spharIdx]);
			m_sphResources[i] = m_dx12.GetTextureByPath(path.c_str());
		}
		else if (m_materials[i].additional.mode == 2)
		{
			if (m_materials[i].additional.spharIdx <= texpath.size())
			{
				auto path = helper::GetStringFromWideString(texpath[m_materials[i].additional.spharIdx]);
				m_spaResources[i] = m_dx12.GetTextureByPath(path.c_str());
			}
		}
		else if (m_materials[i].additional.mode == 3)
		{
				if (m_materials[i].additional.texIdx <= Texnum)
				{
					auto path = helper::GetStringFromWideString(texpath[m_materials[i].additional.texIdx]);
					m_subtextureResources[i] = m_dx12.GetTextureByPath(path.c_str());
				}
		}

		if (!m_materials[i].additional.toonflag)
		{
			if (m_materials[i].additional.toonIdx <= texpath.size())
			{
				auto path = helper::GetStringFromWideString(texpath[m_materials[i].additional.toonIdx]);
				m_toonResources[i] = m_dx12.GetTextureByPath(path.c_str());
			}
		}
		else
		{
			m_toonResources[i] = m_dx12.GetTextureByPath(m_materials[i].additional.defaltetoontex.c_str());
		}


	}

	fclose(fp);
}

bool PMXmodel::getPMXStringUTF16(FILE* file, std::string& outpath)
{
	std::array<wchar_t, 512> wBuffer{};
	uint32_t textSize = 0;
	fread(&textSize, 4, 1, file);
	fread(&wBuffer[0], textSize, 1, file);
	outpath = helper::GetStringFromWideString(wstring(&wBuffer[0], &wBuffer[0] + textSize / 2));
	return true;
}

bool PMXmodel::getPMXStringUTF16(FILE* file, std::wstring& outpath)
{
	std::array<wchar_t, 512> wBuffer{};
	uint32_t textSize = 0;
	fread(&textSize, 4, 1, file);
	fread(&wBuffer[0], textSize, 1, file);
	outpath = wstring(&wBuffer[0], &wBuffer[0] + textSize / 2);
	return true;
}

void PMXmodel::UpdateModelMorf()
{
	m_motion->GetMorphNameFromFrame(m_morphsName, m_morphsWeight);
	for (auto mn : m_morphsName)
	{
		for (auto md : m_morphsData[mn])
		{
			m_vertice[md.vertexIndex].pos = m_originalVertices[md.vertexIndex].pos + (md.posval * m_morphsWeight[mn]);
			m_vertice[md.vertexIndex].uv.x = m_originalVertices[md.vertexIndex].uv.x + (md.uv.x * m_morphsWeight[mn]);
			m_vertice[md.vertexIndex].uv.y = m_originalVertices[md.vertexIndex].uv.y + (md.uv.y * m_morphsWeight[mn]);
		}
	}
	PMXVertice* vertMap = nullptr;
	auto result = m_vb->Map(0, nullptr, (void**)&vertMap);
	std::copy(m_vertice.begin(), m_vertice.end(), vertMap);

}

std::wstring PMXmodel::ChangeCNToJP(std::wstring path)
{
	int num = -1;
	num = path.find(L"面");
	if (num != wstring::npos)
	{
		return path.replace(num, 1, L"顔");
	}
	else
	{
		num = path.find(L"头");
		if (num != wstring::npos)
		{
			path.erase(num, 1);
			return path.replace(num, 1, L"髪");
		}
		else
		{
			num = path.find(L"脸");
			if (num != wstring::npos)
				return path.replace(num, 1, L"顔");
			else
				num = path.find(L"发");
			if (num != wstring::npos)
				return path.replace(num, 1, L"髪");

		}
	}
	return path;
}

