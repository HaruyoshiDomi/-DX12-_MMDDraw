#pragma once
class Model
{
public:
	Model(class Render& dx12);
	~Model();
	virtual void Update(XMMATRIX& martrix) = 0;
	virtual void Draw() = 0;
	ID3D12PipelineState* GetPipelineState() { return m_pipeline.Get(); }
	ID3D12RootSignature* GetRootSignature() { return m_rootSignature.Get(); }

	std::vector<XMMATRIX>* GetBoneMatrix() { return &m_boneMatrieces; }

	void SetMotion(class VMDmotion* motion);
	class VMDmotion* GetMotion() { return m_motion; };

private:



protected:

	struct Transform
	{
		//内部に持ってるXMMATRIXメンバが16バイトアライメントであるため
		//Transformをnewする際には16バイト境界に確保する
		void* operator new(size_t size);
		DirectX::XMMATRIX world;
	};

	struct BoneNode
	{
		uint16_t boneidx;
		uint16_t bonetype;
		uint16_t parentBone;
		uint16_t ikparentBone;
		XMFLOAT3 startPos;
		std::vector<BoneNode*> children;
	};

	Render& m_dx12;

	ComPtr<ID3D12PipelineState> m_pipeline = nullptr;
	ComPtr<ID3D12PipelineState> m_edgepipeline = nullptr;
	ComPtr<ID3D12RootSignature> m_rootSignature = nullptr;

	//頂点関連
	ComPtr<ID3D12Resource> m_vb = nullptr;
	ComPtr<ID3D12Resource> m_ib = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vbView = {};
	D3D12_INDEX_BUFFER_VIEW m_ibView = {};

	ComPtr<ID3D12Resource> m_transformMat = nullptr;		//座標変換行列
	ComPtr<ID3D12DescriptorHeap> m_transformHeap = nullptr;	//座標変換ヒープ


	ComPtr<ID3D12Resource> m_whiteTex = nullptr;
	ComPtr<ID3D12Resource> m_blackTex = nullptr;
	ComPtr<ID3D12Resource> m_gradTex = nullptr;

	ComPtr<ID3D10Blob> m_VSBlob = nullptr;
	ComPtr<ID3D10Blob> m_PSBlob = nullptr;

	Transform m_transform;
	Transform* m_mappedTransform = nullptr;

	XMMATRIX* m_mappedMartrices = nullptr;

	//ボーン関連
	std::vector<XMMATRIX> m_boneMatrieces;
	std::vector<uint32_t> m_kneeIdx;
	std::map<std::string, BoneNode> m_boneNodetable;
	std::vector<std::string> m_boneNameArray;
	std::vector<BoneNode*> m_boneNodeAddressArray;

	ComPtr<ID3D12Resource> m_transformBuff = nullptr;

	ComPtr<ID3D12Resource> m_materialBuff = nullptr;

	class VMDmotion* m_motion;

	std::vector<std::string> m_morphsName;
	std::map<std::string, float> m_morphsWeight;


	HRESULT CreateVS(const std::wstring& filename);
	HRESULT CreatePS(const std::wstring& filename);

	ID3D12Resource* CreateDefaultTexture(size_t width, size_t height);
	ID3D12Resource* CreateWhiteTexture();//白テクスチャの生成
	ID3D12Resource* CreateBlackTexture();//黒テクスチャの生成
	ID3D12Resource* CreateGrayGradationTexture();//グレーテクスチャの生成


	//ルートシグネチャ初期化
	virtual HRESULT CreateRootSignature() = 0;

	//パイプライン初期化
	virtual HRESULT CreateGraphicsPipeline() = 0;


	void RecursiveMatrixMultiply(BoneNode* node, const XMMATRIX& mat, bool flag = false);


public:
	std::map<std::string, BoneNode>* GetBoneNodeTable() { return &m_boneNodetable; }

};

