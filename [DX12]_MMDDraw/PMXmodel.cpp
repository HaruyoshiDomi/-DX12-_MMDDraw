#include "main.h"
#include "Render.h"
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
	m_mappedTransform->world = martrix;

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
		m_dx12.CommandList()->DrawIndexedInstanced(m.indicesNum , 1, idxOffset, 0, 0);
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
		auto result = CreateVS(L"PMXVertexShader.cso");
		if (FAILED(result))
		{
			assert(0);
			return S_FALSE;
		}

		result = CreatePS(L"PMXPixelShader.cso");
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
		{ "EXUV",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TYPE",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "BONENO",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
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
	auto buffSize = sizeof(Transform);
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
	result = m_transformBuff->Map(0, nullptr, (void**)&m_mappedTransform);
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return result;
	}
	*m_mappedTransform = m_transform;

	//ビューの作成
	D3D12_DESCRIPTOR_HEAP_DESC transformDescHeapDesc = {};
	transformDescHeapDesc.NumDescriptors = 1;//とりあえずワールドひとつ
	transformDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	transformDescHeapDesc.NodeMask = 0;

	transformDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	result = m_dx12.Device()->CreateDescriptorHeap(&transformDescHeapDesc, IID_PPV_ARGS(m_transformHeap.ReleaseAndGetAddressOf()));//生成
	if (FAILED(result)) {
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


	uint32_t numVer = 0;

	fread(&numVer, sizeof(numVer), 1, fp);

	enum Type
	{
		BDEF1,
		BDEF2,
		BDEF4,
		SDEF,
	};
	struct Weight
	{

		Type type;
		int born1;
		int	born2;
		int	born3;
		int	born4;
		float weight1;
		float weight2;
		float weight3;
		float weight4;
		XMFLOAT3 c;
		XMFLOAT3 r0;
		XMFLOAT3 r1;
	};

	std::vector<Weight> weight(numVer);

	struct PMXVertice
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		XMFLOAT4 additionaluv[4];
		uint8_t type;
		int born1;
		int	born2;
		int	born3;
		int	born4;
		float weight1;
		float weight2;
		float weight3;
		float weight4;
		XMFLOAT3 c;
		XMFLOAT3 r0;
		XMFLOAT3 r1;

		float edgenif;

	};


	std::vector<PMXVertice> vertice(numVer);
	std::vector<Vertices> vertices(numVer);
	// ボーンウェイト

	for (int i = 0; i < numVer; i++)
	{
		fread(&vertice[i].pos, sizeof(XMFLOAT3), 1, fp);
		fread(&vertice[i].normal, sizeof(XMFLOAT3), 1, fp);
		fread(&vertice[i].uv, sizeof(XMFLOAT2), 1, fp);
		vertices[i].pos = vertice[i].pos;
		vertices[i].normal = vertice[i].normal;
		vertices[i].uv = vertice[i].uv;
		if (hederData[NUMBER_OF_ADD_UV] != 0)
		{
			for (int j = 0; j < hederData[NUMBER_OF_ADD_UV]; ++j)
			{
				fread(&vertice[i].additionaluv[j], sizeof(XMFLOAT4), 1, fp);
			}
		}
		uint8_t weightMethot = 0;
		fread(&weightMethot, sizeof(weightMethot), 1, fp);
		switch (weightMethot)
		{
		case Type::BDEF1:
			weight[i].type = Type::BDEF1;
			fread(&vertice[i].born1, hederData[BONE_INDEX_SIZE], 1, fp);
			vertice[i].born2 = -1;
			vertice[i].born3 = -1;
			vertice[i].born4 = -1;
			vertice[i].weight1 = 1.0f;
			break;
		case Type::BDEF2:
			weight[i].type = Type::BDEF2;
			fread(&vertice[i].born1, hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&vertice[i].born2, hederData[BONE_INDEX_SIZE], 1, fp);
			vertice[i].born3 = -1;
			vertice[i].born4 = -1;
			fread(&vertice[i].weight1, sizeof(float), 1, fp);
			vertice[i].weight2 = 1.0f - vertice[i].weight1;
			break;
		case Type::BDEF4:
			weight[i].type = Type::BDEF4;
			fread(&vertice[i].born1, hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&vertice[i].born2, hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&vertice[i].born3, hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&vertice[i].born4, hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&vertice[i].weight1, sizeof(float), 1, fp);
			fread(&vertice[i].weight2, sizeof(float), 1, fp);
			fread(&vertice[i].weight3, sizeof(float), 1, fp);
			fread(&vertice[i].weight4, sizeof(float), 1, fp);
			break;
		case Type::SDEF:
			weight[i].type = Type::SDEF;
			fread(&vertice[i].born1, hederData[BONE_INDEX_SIZE], 1, fp);
			fread(&vertice[i].born2, hederData[BONE_INDEX_SIZE], 1, fp);
			vertice[i].born3 = -1;
			vertice[i].born4 = -1;
			fread(&vertice[i].weight1, sizeof(float), 1, fp);
			vertice[i].weight2 = 1.0f - vertice[i].weight1;
			fread(&vertice[i].c, sizeof(XMFLOAT3), 1, fp);
			fread(&vertice[i].r0, sizeof(XMFLOAT3), 1, fp);
			fread(&vertice[i].r1, sizeof(XMFLOAT3), 1, fp);
			break;
		}

		fread(&vertice[i].edgenif, sizeof(float), 1, fp);
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
		int parentidx;
		int transformlv;
		int childenidx;
		uint16_t flag;
	};

	fread(&bonenum, sizeof(bonenum), 1, fp);
	m_boneMatrieces.resize(bonenum);
	std::vector<PMXbone> bone(bonenum);
	//ボーンを初期化
	std::fill(m_boneMatrieces.begin(), m_boneMatrieces.end(), XMMatrixIdentity());
	//{
	//	enum BoneFlagMask
	//	{
	//		ACCESS_POINT = 0x0001,
	//		IK = 0x0020,
	//		IMPART_ROTATION = 0x0100,
	//		IMPART_TRANSLATION = 0x0200,
	//		AXIS_FIXING = 0x0400,
	//		LOCAL_AXIS = 0x0800,
	//		EXTERNAL_PARENT_TRANS = 0x2000,
	//	};
	//	//for (int i = 0; i < bonenum; i++)
	//	//{
	//	//	getPMXStringUTF16(fp, bone[i].name);
	//	//	getPMXStringUTF16(fp, bone[i].name_eng);
	//	//	fread(&bone[i].pos, sizeof(bone[i].pos), 1, fp);
	//	//	fread(&bone[i].parentidx, hederData[BONE_INDEX_SIZE], 1, fp);
	//	//	if (bone[i].parentidx >= bonenum)
	//	//	{
	//	//		bone[i].parentidx = -1;
	//	//	}
	//	//	fread(&bone[i].transformlv, sizeof(bone[i].transformlv), 1, fp);
	//	//	fread(&bone[i].flag, 2, 1, fp);

	//	//	if (bone[i].flag & ACCESS_POINT)
	//	//	{
	//	//		fread(&bone[i].childenidx, hederData[BONE_INDEX_SIZE], 1, fp);
	//	//		if (bone[i].childenidx >= bonenum)
	//	//		{
	//	//			bone[i].childenidx = -1;
	//	//		}
	//	//	}
	//	//	else
	//	//	{
	//	//		bone[i].childenidx = -1;
	//	//		fread(&bone[i].coordofset, sizeof(bone[i].coordofset), 1, fp);
	//	//	}

	//	//	if ((bone[i].flag & IMPART_ROTATION) || (bone[i].flag & IMPART_TRANSLATION))
	//	//	{

	//	//	}

	//	//	m_boneNodetable[bone[i].name].startPos = bone[i].pos;
	//	//	m_boneNodetable[bone[i].name].boneidx = bone[i].parentidx;

	//	//}

	//}


	//バッファ書き込み

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(vertice.size() * sizeof(vertice[0]));

	auto result = m_dx12.Device()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_vb.ReleaseAndGetAddressOf()));

	PMXVertice* vertMap = nullptr;
	result = m_vb->Map(0, nullptr, (void**)&vertMap);
	std::copy(vertice.begin(), vertice.end(), vertMap);
	m_vb->Unmap(0, nullptr);

	m_vbView.BufferLocation = m_vb->GetGPUVirtualAddress();//バッファの仮想アドレス
	m_vbView.SizeInBytes = vertices.size() * sizeof(PMXVertice);//全バイト数
	m_vbView.StrideInBytes = sizeof(vertice[0]);//1頂点あたりのバイト数

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

bool PMXmodel::getPMXStringUTF16(FILE* helper, std::string& outpath)
{
	std::array<wchar_t, 512> wBuffer{};
	uint32_t textSize = 0;
	fread(&textSize, 4, 1, helper);
	fread(&wBuffer[0], textSize, 1, helper);
	outpath = helper::GetStringFromWideString(wstring(&wBuffer[0], &wBuffer[0] + textSize / 2));
	return true;
}

bool PMXmodel::getPMXStringUTF16(FILE* helper, std::wstring& outpath)
{
	std::array<wchar_t, 512> wBuffer{};
	uint32_t textSize = 0;
	fread(&textSize, 4, 1, helper);
	fread(&wBuffer[0], textSize, 1, helper);
	outpath = wstring(&wBuffer[0], &wBuffer[0] + textSize / 2);
	return true;
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

