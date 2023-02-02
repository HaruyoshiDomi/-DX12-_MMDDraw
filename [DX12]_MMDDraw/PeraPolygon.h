#pragma once
#include "Object.h"
class PeraPolygon : public Object
{
public:
	PeraPolygon() = delete;
	PeraPolygon(class Render& dx12);
	~PeraPolygon();

	void Init()override;
	void Uninit()override;
	void Draw()override;

protected:
	struct PeraVertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
	void ActorUpdate() {}

private:

	class Render& m_dx12;

	PeraVertex m_pv[4];
	ComPtr<ID3D12PipelineState> m_pipeline;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexView;

	HRESULT CreateRootSignature();
	HRESULT CreateGraphicsPipeline();


};

