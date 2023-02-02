#include "main.h"
#include "Render.h"
#include "PeraPolygon.h"



PeraPolygon::PeraPolygon(Render& dx12) : m_dx12(dx12)
{
	CreateRootSignature();
	CreateGraphicsPipeline();
}

PeraPolygon::~PeraPolygon()
{
}
void PeraPolygon::Init()
{
	m_pv[0] = { { -1,-1, 0.1 }, {0, 1} }; //左下
	m_pv[1] = { { -1, 1, 0.1 }, {0, 0} }; //左上
	m_pv[2] = { {  1,-1, 0.1 }, {1, 1} }; //右下
	m_pv[3] = { {  1, 1, 0.1 }, {1, 0} }; //右上

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(m_pv));

	auto result = m_dx12.Device()->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_vertexBuffer.ReleaseAndGetAddressOf())
	);

	if (!helper::CheckResult(result))
		assert(0);

	PeraVertex* map = nullptr;
	m_vertexBuffer->Map(0, nullptr, (void**)&map);
	copy(begin(m_pv), end(m_pv), map);
	m_vertexBuffer->Unmap(0, nullptr);

	m_vertexView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexView.SizeInBytes = sizeof(m_pv);
	m_vertexView.StrideInBytes = sizeof(PeraVertex);

}

void PeraPolygon::Uninit()
{
}

void PeraPolygon::Draw()
{
	m_dx12.CommandList()->SetGraphicsRootSignature(m_rootSignature.Get());
	m_dx12.CommandList()->SetPipelineState(m_pipeline.Get());
	m_dx12.CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_dx12.CommandList()->IASetVertexBuffers(0, 1, &m_vertexView);
	m_dx12.CommandList()->DrawInstanced(4, 1, 0, 0);
}

HRESULT PeraPolygon::CreateRootSignature()
{
	CD3DX12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.NumParameters = 0;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> rsBlob = nullptr;
	ComPtr<ID3DBlob> errBlob = nullptr;

	auto result = D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		rsBlob.ReleaseAndGetAddressOf(),
		errBlob.ReleaseAndGetAddressOf()
	);
	if (!helper::CheckResult(result))
		return S_FALSE;

	result = m_dx12.Device()->CreateRootSignature(
		0, rsBlob->GetBufferPointer(),
		rsBlob->GetBufferSize(),
		IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf()));
	if (!helper::CheckResult(result))
		return S_FALSE;


	return S_OK;
}
HRESULT PeraPolygon::CreateGraphicsPipeline()
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};

	CD3DX12_PIPELINE_STATE_STREAM2(desc);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gppipeline = desc.GraphicsDescV0();
	ComPtr<ID3DBlob> vs;
	ComPtr<ID3DBlob> ps;

	if (FAILED(D3DReadFileToBlob(L"PeraVS.cso", vs.GetAddressOf())))
	{
		assert(0);
		return S_FALSE;
	}
	if (FAILED(D3DReadFileToBlob(L"PeraPS.cso", ps.GetAddressOf())))
	{
		assert(0);
		return S_FALSE;
	}

	for (int i = 0; i < _countof(gppipeline.BlendState.RenderTarget); ++i)
	{
		//見やすくするため変数化
		auto& rt = gppipeline.BlendState.RenderTarget[i];
		rt.BlendEnable = true;
		rt.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		rt.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		rt.BlendOp = D3D12_BLEND_OP_ADD;
		rt.SrcBlendAlpha = D3D12_BLEND_ONE;
		rt.DestBlendAlpha = D3D12_BLEND_ONE;
		rt.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		rt.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}
	gppipeline.VS = CD3DX12_SHADER_BYTECODE(vs.Get());
	gppipeline.PS = CD3DX12_SHADER_BYTECODE(ps.Get());
	gppipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//三角形で構成
	gppipeline.InputLayout.pInputElementDescs = inputLayout;						//レイアウト先頭アドレス
	gppipeline.InputLayout.NumElements = _countof(inputLayout);					//レイアウト配列数
	gppipeline.NumRenderTargets = 1;
	gppipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;						//0～1に正規化されたRGBA
	gppipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gppipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gppipeline.pRootSignature = m_rootSignature.Get();

	auto result = m_dx12.Device()->CreateGraphicsPipelineState(&gppipeline, IID_PPV_ARGS(m_pipeline.ReleaseAndGetAddressOf()));
	return S_OK;
}
