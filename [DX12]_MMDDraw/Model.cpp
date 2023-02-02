#include "main.h"
#include "Render.h"
#include "VMDmotion.h"
#include "Model.h"

Model::Model(Render& dx12) : m_dx12(dx12)
{
	m_whiteTex = CreateWhiteTexture();
	m_blackTex = CreateBlackTexture();
	m_gradTex = CreateGrayGradationTexture();
}

Model::~Model()
{
}

void Model::SetMotion(VMDmotion* motion)
{
	if (!m_motion)
	{
		m_motion = motion;
		motion->m_starTime = timeGetTime();
	}
	m_motion->m_model = this;
	motion->SetNowPose();
	RecursiveMatrixMultiply(&m_boneNodetable["センター"], XMMatrixIdentity());
	copy(m_boneMatrieces.begin(), m_boneMatrieces.end(), m_mappedMartrices + 1);
	
}

HRESULT Model::CreateVS(const std::wstring& filename)
{
	if (FAILED(D3DReadFileToBlob(filename.c_str(), m_VSBlob.GetAddressOf())))
	{
		assert(0);
		return S_FALSE;
	}
	return S_OK;
}

HRESULT Model::CreatePS(const std::wstring& filename)
{
	if (FAILED(D3DReadFileToBlob(filename.c_str(), m_PSBlob.GetAddressOf())))
	{
		assert(0);
		return S_FALSE;
	}
	return S_OK;
}

ID3D12Resource* Model::CreateDefaultTexture(size_t width, size_t height)
{
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	ID3D12Resource* buff = nullptr;
	auto result = m_dx12.Device()->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&buff)
	);
	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return nullptr;
	}

	return buff;
}

ID3D12Resource* Model::CreateWhiteTexture()
{
	ID3D12Resource* whiteBuff = CreateDefaultTexture(4, 4);

	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);

	auto result = whiteBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, data.size());
	assert(SUCCEEDED(result));
	return whiteBuff;
}

ID3D12Resource* Model::CreateBlackTexture()
{
	ID3D12Resource* blackBuff = CreateDefaultTexture(4, 4);
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0x00);

	auto result = blackBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, data.size());
	assert(SUCCEEDED(result));
	return blackBuff;
}

ID3D12Resource* Model::CreateGrayGradationTexture()
{
	ID3D12Resource* gradBuff = CreateDefaultTexture(4, 256);
	//上が白くて下が黒いテクスチャデータを作成
	std::vector<unsigned int> data(4 * 256);
	auto it = data.begin();
	unsigned int c = 0xff;
	for (; it != data.end(); it += 4) 
	{
		auto col = (0xff << 24) | RGB(c, c, c);//RGBAが逆並びしているためRGBマクロと0xff<<24を用いて表す。
		std::fill(it, it + 4, col);
		--c;
	}

	auto result = gradBuff->WriteToSubresource(0, nullptr, data.data(), 4 * sizeof(unsigned int), sizeof(unsigned int) * data.size());
	assert(SUCCEEDED(result));
	return gradBuff;
}

void Model::RecursiveMatrixMultiply(BoneNode* node, const XMMATRIX& mat, bool flag)
{
	m_boneMatrieces[node->boneidx] *= mat;

	for (auto& cnode : node->children)
	{
		RecursiveMatrixMultiply(cnode, m_boneMatrieces[node->boneidx]);
	}
}


void* Model::Transform::operator new(size_t size)
{
	return _aligned_malloc(size, 16);
}
