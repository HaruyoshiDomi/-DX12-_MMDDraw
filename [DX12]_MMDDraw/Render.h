#pragma once
namespace helper
{
	std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath);
	std::wstring GetTexturePathFromModelAndTexPath(const std::wstring& modelPath, const wchar_t* texPath);

	///�t�@�C��������g���q���擾����
	///@param path �Ώۂ̃p�X������
	///@return �g���q
	std::string GetExtension(const std::string& path);

	///�t�@�C��������g���q���擾����(���C�h������)
	///@param path �Ώۂ̃p�X������
	///@return �g���q
	std::wstring GetExtension(const std::wstring& path);

	///�e�N�X�`���̃p�X���Z�p���[�^�����ŕ�������
	///@param path �Ώۂ̃p�X������
	///@param splitter ��؂蕶��
	///@return �����O��̕�����y�A
	pair<string, string> SplitFileName(const std::string& path, const char splitter = '*');

	pair<wstring, wstring> SplitFileName(const std::wstring& path, const char splitter = '*');

	///string(�}���`�o�C�g������)����wstring(���C�h������)�𓾂�
	///@param str �}���`�o�C�g������
	///@return �ϊ����ꂽ���C�h������
	std::wstring GetWideStringFromString(const std::string& str);

	///wstring(���C�h������)����string(�}���`�o�C�g������)�𓾂�
	///@param str ���C�h������
	///@return �ϊ����ꂽ�}���`�o�C�g������
	std::string GetStringFromWideString(const std::wstring& wstr);


	extern bool CheckResult(HRESULT& result, ID3DBlob* errBlob = nullptr);

}

class Render
{
public:
	HRESULT Init(const HWND& hwnd);
	void Update();
	void Draw();
	void Uninit();

	static Render* Instance();

	ComPtr<ID3D12Resource> GetTextureByPath(const char* texpath);
	ComPtr<ID3D12Device> Device() { return m_dev; }
	ComPtr<ID3D12GraphicsCommandList> CommandList() { return m_cmdList; }
	ComPtr<ID3D12Resource> GetShadowTexture();

	void SetSceneView();
	class Model* GetLodedModel(int num = 0);
	int GetCharacterNum() { return m_modelTabel.size(); }
	void CameraUpdate();

	~Render();

private:
	Render();
	Render(const Render&) = delete;
	void operator=(const Render&) = delete;

	static Render* m_instance;



	ComPtr<IDXGIFactory4> m_dxgifactory = nullptr;
	ComPtr<IDXGISwapChain3> m_swapchain = nullptr;

	ComPtr<ID3D12Device> m_dev = nullptr;
	ComPtr<ID3D12CommandAllocator> m_cmdAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_cmdList = nullptr;
	ComPtr<ID3D12CommandQueue> m_cmdQueue = nullptr;

	std::vector<ID3D12Resource*> m_backBuffers = {};
	ComPtr<ID3D12Resource> m_depthBuffer = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeaps = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap = nullptr;
	std::unique_ptr<D3D12_VIEWPORT> m_viewport;
	std::unique_ptr<D3D12_RECT> m_scissorrect;

	ComPtr<ID3D12Resource> m_sceneConstBuff = nullptr;
	ComPtr<ID3D12Resource> m_peraResource = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_peraRTVHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_peraSRVHeap = nullptr;

	struct SceneData
	{
		XMMATRIX view;//�r���[�v���W�F�N�V�����s��
		XMMATRIX proj;//
		XMMATRIX shadow;
		XMFLOAT3 eye;//���_���W
	};

	XMFLOAT3 m_eye;
	XMFLOAT3 m_target;
	XMFLOAT3 m_up;
	XMFLOAT3 m_parallelLightVec;
	XMFLOAT2 m_oldMousePos;
	SceneData* m_mappedSceneData = nullptr;
	ComPtr<ID3D12DescriptorHeap> m_sceneDescHeap = nullptr;

	ComPtr<ID3D12Fence> m_fence = nullptr;
	UINT64 m_fenceVal = 0;

	using LoadLamda_t = std::function<HRESULT(const std::wstring& path, TexMetadata*, ScratchImage&)>;
	std::map<std::string, LoadLamda_t> m_loadLamdatable = {};
	std::unordered_map<std::string, ComPtr<ID3D12Resource>> m_textureTable;
	std::vector<class Model*> m_modelTabel = {};
	XMFLOAT4 m_clarColer = {}; //�����_�����O���̃N���A�J���[

	void CreateTextureLoaderTable();

	ID3D12Resource* CreateTextureFromFile(const char* Texpath);
	HRESULT InitializeDXGIDevice();
	HRESULT InitializeCommand();
	HRESULT CreateSwapChain(const HWND& hwnd);
	HRESULT CreateFinalRenderTargets();
	HRESULT CreateSceneView();
	HRESULT CreateDepthStencilView();

	bool CreatePeraResourcesAndView();

	void BeginDraw();
	void EndDraw();

	void InitCamera(DXGI_SWAP_CHAIN_DESC1& desc);
	
};