#include "main.h"
#include <d3dx12.h>
#include "Manager.h"
#include "input.h"
#include "Mouse.h"
#include "PMDmodel.h"
#include "PMXmodel.h"
#include "Character.h"
#include "VMDmotion.h"
#include "XMFLOAT_Helper.h"
#include"imgui\imgui.h"
#include"imgui\imgui_impl_win32.h"
#include"imgui\imgui_impl_dx12.h"
#include "Render.h"

Render* Render::m_instance = nullptr;

HRESULT Render::Init(const HWND& hwnd)
{
	if (FAILED(InitializeDXGIDevice()))
	{
		assert(0);
		return S_FALSE;
	}
	if (FAILED(InitializeCommand()))
	{
		assert(0);
		return S_FALSE;
	}
	if (FAILED(CreateSwapChain(hwnd)))
	{
		assert(0);
		return S_FALSE;
	}
	if (FAILED(CreateFinalRenderTargets()))
	{
		assert(0);
		return S_FALSE;
	}
	if (FAILED(CreateSceneView()))
	{
		assert(0);
		return S_FALSE;
	}

	CreateTextureLoaderTable();

	if (FAILED(CreateDepthStencilView()))
	{
		assert(0);
		return S_FALSE;
	}

	
	if(FAILED(m_dev->CreateFence(m_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf()))))
	{
		assert(0);
		return S_FALSE;
	}

	m_heapForImgui = CreateDescriptorHeapForImgui();
	if(!m_heapForImgui)
		return S_FALSE;

	m_modelTabel.push_back(new PMDmodel("asset/Model/PMD/�����䂩��_��_ver1.0/�����䂩��_��_ver1.0.pmd", *this));
	m_modelTabel.push_back(new PMDmodel("asset/Model/PMD/Lat���~�NVer2.31/Lat���~�NVer2.31_Normal.pmd", *this));
	m_modelTabel.push_back(new PMXmodel("asset/Model//PMX/�䂩��ver7/�䂩��ver7.pmx", *this));
	for (int i = 0; i < m_modelTabel.size(); i++)
	{
		m_modelNameTabel.push_back(m_modelTabel[i]->GetName());
	}
	Manager::GetInstance()->Init();

	return S_OK;
}

void Render::Update()
{
	CameraUpdate();
	Manager::GetInstance()->Update();
}
void Render::Draw()
{
	BeginDraw();

	Manager::GetInstance()->Draw();

	ImguiDrawing();
	EndDraw();
	m_swapchain->Present(1, 0);
}

void Render::Uninit()
{
	delete m_instance;
	m_instance = nullptr;
	Manager::GetInstance()->Uninit();
	for (auto& model : m_modelTabel)
	{
		delete model;
	}
	m_modelTabel.clear();
	m_modelTabel.shrink_to_fit();
}

Render* Render::Instance()
{
	if (!m_instance)
		m_instance = new Render();
	return m_instance;
}


void Render::CreateTextureLoaderTable()
{
	m_loadLamdatable["sph"] = m_loadLamdatable["spa"] = m_loadLamdatable["bmp"] = m_loadLamdatable["png"] = m_loadLamdatable["jpg"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT 
	{
		return LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, meta, img);
	};

	m_loadLamdatable["tga"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT 
	{
		return LoadFromTGAFile(path.c_str(), meta, img);
	};

	m_loadLamdatable["dds"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT 
	{
		return LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, meta, img);
	};
}

ID3D12Resource* Render::CreateTextureFromFile(const char* Texpath)
{
	string texPath = Texpath;
	//�e�N�X�`���̃��[�h
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	auto wtexpath = helper::GetWideStringFromString(texPath);//�e�N�X�`���̃t�@�C���p�X
	auto ext = helper::GetExtension(texPath);//�g���q���擾
	if (FAILED(m_loadLamdatable[ext](wtexpath, &metadata, scratchImg)))
	{
		return nullptr;
	}

	auto img = scratchImg.GetImage(0, 0, 0);//���f�[�^���o

	//WriteToSubresource�œ]������p�̃q�[�v�ݒ�
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, metadata.width, metadata.height, metadata.arraySize, metadata.mipLevels);

	ID3D12Resource* texbuff = nullptr;
	if (FAILED(m_dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texbuff))))
	{
		return nullptr;
	}

	if (FAILED(texbuff->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch)))
	{
		return nullptr;
	}

	m_textureTable[Texpath] = texbuff;
	return texbuff;
}

ComPtr<ID3D12Resource> Render::GetTextureByPath(const char* texpath)
{
	auto it = m_textureTable.find(texpath);
	if (it != m_textureTable.end())
	{
		return m_textureTable[texpath];
	}
	else
	{
		return ComPtr<ID3D12Resource>(CreateTextureFromFile(texpath));
	}
}


HRESULT Render::InitializeDXGIDevice()
{
	UINT flagsDXGI = 0;
	flagsDXGI |= DXGI_CREATE_FACTORY_DEBUG;
	if (FAILED(CreateDXGIFactory2(flagsDXGI, IID_PPV_ARGS(m_dxgifactory.ReleaseAndGetAddressOf()))))
		return S_FALSE;

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	std::vector<IDXGIAdapter*> adapters;		//�A�_�v�^�[�̗񋓗p
	IDXGIAdapter* tmpAdapter = nullptr;			//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������

	for (int i = 0; m_dxgifactory.Get()->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);		//�A�_�v�^�[�����I�u�W�F�N�g�擾

		std::wstring strDesc = adesc.Description;

		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	//Direct3D�f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featurelevel;

	for (auto lv : levels)
	{
		if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(m_dev.ReleaseAndGetAddressOf())) == S_OK)
		{
			featurelevel = lv;
			return S_OK;
		}
	}

	return S_FALSE;
}

HRESULT Render::InitializeCommand()
{
	if (FAILED(m_dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_cmdAllocator.ReleaseAndGetAddressOf()))))
	{
		assert(0);
		return S_FALSE;
	}
	if (FAILED(m_dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator.Get(), nullptr, IID_PPV_ARGS(m_cmdList.ReleaseAndGetAddressOf()))))
	{
		assert(0);
		return S_FALSE;
	}

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;				//�^�C���A�E�g�Ȃ�
	cmdQueueDesc.NodeMask = 0;										//�A�_�v�^�[��1�����g��Ȃ��Ƃ���0�ł悢
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	//�v���C�I���e�B�͓��Ɏw��Ȃ�
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;				//�R�}���h���X�g�ƍ��킹��

	//�L���[����
	if (FAILED(m_dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(m_cmdQueue.ReleaseAndGetAddressOf()))))
	{
		assert(0);
		return S_FALSE;
	}

	return S_OK;
}

HRESULT Render::CreateSwapChain(const HWND& hwnd)
{
	RECT rc = {};
	::GetWindowRect(hwnd, &rc);

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	swapchainDesc.Width = SCREEN_WIDTH;
	swapchainDesc.Height = SCREEN_HEIGHT;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.BufferCount = 2;
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;					//�o�b�N�o�b�t�@�[�͐L�яk�݉\
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		//�t���b�v��͑��₩�ɔj��
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;			//���Ɏw��Ȃ�
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	//�E�B���h�E���[�h�������t���X�N���[���؂�ւ��\


	return 	m_dxgifactory->CreateSwapChainForHwnd(m_cmdQueue.Get(), hwnd, &swapchainDesc, nullptr, nullptr, (IDXGISwapChain1**)m_swapchain.ReleaseAndGetAddressOf());
}

HRESULT Render::CreateFinalRenderTargets()
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	if (FAILED(m_swapchain->GetDesc1(&desc)))
		return S_FALSE;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		//�����_�[�^�[�Q�b�g�r���[�Ȃ̂�RTV
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;						//���\��2��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	//���Ɏw��Ȃ�

	if (FAILED(m_dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_rtvHeaps.ReleaseAndGetAddressOf()))))
		return S_FALSE;

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	if (FAILED(m_swapchain->GetDesc(&swcDesc)))
		return S_FALSE;	
	m_backBuffers.resize(swcDesc.BufferCount);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	//SRGB �����_�[�^�[�Q�b�g�r���[�ݒ�
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (int idx = 0; idx < (int)swcDesc.BufferCount; ++idx)
	{
		auto result = m_swapchain->GetBuffer(idx, IID_PPV_ARGS(&m_backBuffers[idx]));
		assert(SUCCEEDED(result));
		rtvDesc.Format = m_backBuffers[idx]->GetDesc().Format;
		m_dev->CreateRenderTargetView(m_backBuffers[idx], &rtvDesc, handle);
		handle.ptr += m_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
	
	m_viewport.reset(new CD3DX12_VIEWPORT(m_backBuffers[0]));
	m_scissorrect.reset(new CD3DX12_RECT(0, 0, desc.Width, desc.Height));

	return S_OK;
}

HRESULT Render::CreateSceneView()
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = m_swapchain->GetDesc1(&desc);
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + 0xff) & ~0xff);
	//�萔�o�b�t�@�쐬
	result = m_dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(m_sceneConstBuff.ReleaseAndGetAddressOf())
	);

	if (FAILED(result)) 
	{
		assert(SUCCEEDED(result));
		return result;
	}

	m_mappedSceneData = nullptr;//�}�b�v��������|�C���^
	result = m_sceneConstBuff->Map(0, nullptr, (void**)&m_mappedSceneData);//�}�b�v

	InitCamera(desc);

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;//�V�F�[�_���猩����悤��
	descHeapDesc.NodeMask = 0;//�}�X�N��0
	descHeapDesc.NumDescriptors = 1;//
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���
	result = m_dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(m_sceneDescHeap.ReleaseAndGetAddressOf()));//����

	////�f�X�N���v�^�̐擪�n���h�����擾���Ă���
	auto heapHandle = m_sceneDescHeap->GetCPUDescriptorHandleForHeapStart();
	
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_sceneConstBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = m_sceneConstBuff->GetDesc().Width;
	//�萔�o�b�t�@�r���[�̍쐬
	m_dev->CreateConstantBufferView(&cbvDesc, heapHandle);
	return result;
}

HRESULT Render::CreateDepthStencilView()
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	auto result = m_swapchain->GetDesc1(&desc);

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resdesc.DepthOrArraySize = 1;
	resdesc.Width = desc.Width;
	resdesc.Height = desc.Height;
	resdesc.Format = DXGI_FORMAT_D32_FLOAT;
	resdesc.SampleDesc.Count = 1;
	resdesc.SampleDesc.Quality = 0;
	resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resdesc.MipLevels = 1;
	resdesc.Alignment = 0;

	//�f�v�X�p�q�[�v�v���p�e�B
	auto depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	CD3DX12_CLEAR_VALUE depthClearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	result = m_dev->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
		&depthClearValue,
		IID_PPV_ARGS(m_depthBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(result)) 
	{
		//�G���[����
		return result;
	}

	//�[�x�̂��߂̃f�X�N���v�^�q�[�v�쐬
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};//�[�x�Ɏg����Ƃ��������킩��΂���
	dsvHeapDesc.NumDescriptors = 1;//�[�x�r���[1�̂�
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//�f�v�X�X�e���V���r���[�Ƃ��Ďg��
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;


	result = m_dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.ReleaseAndGetAddressOf()));

	//�[�x�r���[�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//�f�v�X�l��32bit�g�p
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//�t���O�͓��ɂȂ�
	m_dev->CreateDepthStencilView(m_depthBuffer.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return S_OK;
}

bool Render::CreatePeraResourcesAndView()
{
	//�쐬�ς݂̃q�[�v���𗘗p����
	auto heapdesc = m_rtvHeaps->GetDesc();

	//�g���Ă���o�b�N�o�b�t�@�[�̏��𗘗p����
	auto& bbuff = m_backBuffers[0];
	auto resdesc = bbuff->GetDesc();

	D3D12_HEAP_PROPERTIES heapprop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//�����_�����O���̃N���A�l�Ɠ���
	float clearColor[4] = { m_clarColer.x,m_clarColer.y,m_clarColer.z,m_clarColer.w };
	D3D12_CLEAR_VALUE clearvalue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_R8G8B8A8_UNORM, clearColor);

	auto result = m_dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearvalue,
		IID_PPV_ARGS(m_peraResource.ReleaseAndGetAddressOf())
	);

	if (!helper::CheckResult(result))
	{
		assert(0);
		return false;
	}

	//RTV�p�q�[�v�쐬
	heapdesc.NumDescriptors = 1;
	result = m_dev->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(m_peraRTVHeap.ReleaseAndGetAddressOf()));
	if (!helper::CheckResult(result))
	{
		assert(0);
		return false;
	}

	//RTV�쐬
	D3D12_RENDER_TARGET_VIEW_DESC rtvdesc = {};
	rtvdesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	auto handle = m_peraRTVHeap->GetCPUDescriptorHandleForHeapStart();

	m_dev->CreateRenderTargetView(m_peraResource.Get(), &rtvdesc, m_peraRTVHeap->GetCPUDescriptorHandleForHeapStart());

	//SRV�p�q�[�v�쐬
	heapdesc.NumDescriptors = 1;
	heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	result = m_dev->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(m_peraSRVHeap.ReleaseAndGetAddressOf()));
	if (!helper::CheckResult(result))
	{
		assert(0);
		return false;
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
	srvdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvdesc.Format = rtvdesc.Format;
	srvdesc.Texture2D.MipLevels = 1;
	srvdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//SRV�쐬
	m_dev->CreateShaderResourceView(m_peraResource.Get(), &srvdesc, m_peraSRVHeap->GetCPUDescriptorHandleForHeapStart());

	return true;
}
void Render::BeginDraw()
{
	auto bbIdx = m_swapchain->GetCurrentBackBufferIndex();
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[bbIdx],
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_cmdList->ResourceBarrier(1, &barrier);


	//�����_�[�^�[�Q�b�g���w��
	auto rtvH = m_rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += bbIdx * m_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//�[�x���w��
	auto dsvH = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	m_cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
	m_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	//��ʃN���A
	float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };
	m_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	//�r���[�|�[�g�A�V�U�[��`�̃Z�b�g
	m_cmdList->RSSetViewports(1, m_viewport.get());
	m_cmdList->RSSetScissorRects(1, m_scissorrect.get());
}

void Render::EndDraw()
{
	auto bbIdx = m_swapchain->GetCurrentBackBufferIndex();

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[bbIdx],
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	m_cmdList->ResourceBarrier(1, &barrier);

	//���߂̃N���[�Y
	m_cmdList->Close();

	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlists[] = { m_cmdList.Get() };
	m_cmdQueue->ExecuteCommandLists(1, cmdlists);
	////�҂�
	m_cmdQueue->Signal(m_fence.Get(), ++m_fenceVal);

	if (m_fence->GetCompletedValue() < m_fenceVal) 
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);
		m_fence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	m_cmdAllocator->Reset();//�L���[���N���A
	m_cmdList->Reset(m_cmdAllocator.Get(), nullptr);//�ĂуR�}���h���X�g�����߂鏀��
}
void Render::ImguiDrawing()
{
	
	auto crt = Manager::GetInstance()->GetObjects<Character>();
	static int modelnum = 0;
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Setting");
	ImGui::SetWindowSize(ImVec2(400, 200), ImGuiCond_::ImGuiCond_FirstUseEver);
	{
		for (int i = 0; i < m_modelNameTabel.size(); i++)
		{
			ImGui::RadioButton(m_modelNameTabel[i].c_str(), &modelnum, i);
		}
	}

	ImGui::SliderFloat("Lighting_X", &m_parallelLightVec.x, -30.0f, 30.0f);
	ImGui::SliderFloat("Lighting_Z", &m_parallelLightVec.z, -30.0f, 30.0f);
	ImGui::SliderInt("ChangeModel", &modelnum, 0, m_modelTabel.size() - 1);
	ImGui::BeginMainMenuBar();
	for (auto c : crt)
	{
		if (!c->GetMotionFlag())
		{
			if (ImGui::Button("AutoRotation_Y"))
				c->AutoRotation();
			ImGui::SameLine();
			if (ImGui::Button("ResetRotate"))
				c->ResetRotate();
			if (ImGui::Button("SetMotion"))
				c->SetMotion();
		}
		else
		{
			int f = c->GetMotion()->GetNowFrame();
			auto motion = c->GetMotion();
			if (ImGui::SliderInt("NowMotionFrame", &f, 0, c->GetMotion()->GetFinalFrame()))
				c->GetMotion()->SetNewMotionFrame(f);
			if (ImGui::Button("PLAY MOTION"))
				c->MotionPlayAndStop();
			if (ImGui::Button("RESET MOTION"))
				c->ResetMotion();
		}
		c->SetModel(modelnum);
	}
	ImGui::EndMainMenuBar();

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
	ImGui::Render();
	m_cmdList->SetDescriptorHeaps(1, m_heapForImgui.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_cmdList.Get());
}

void Render::InitCamera(DXGI_SWAP_CHAIN_DESC1& desc)
{
	m_eye = XMFLOAT3(0, 15, -15);
	m_target = XMFLOAT3(0, 15, 0);
	m_up = XMFLOAT3(0, 1, 0);
	m_mappedSceneData->light = { m_parallelLightVec.x, m_parallelLightVec.y, m_parallelLightVec.z, 0 };
	m_mappedSceneData->view = XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_target), XMLoadFloat3(&m_up));
	m_mappedSceneData->proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,//��p��45��
		static_cast<float>(desc.Width) / static_cast<float>(desc.Height),//�A�X��
		0.1f,//�߂���
		1000.0f//������
	);

	XMFLOAT4 planenormalvec(0, 1, 0, 0);
	m_mappedSceneData->eye = m_eye;
	m_mappedSceneData->shadow = XMMatrixShadow(XMLoadFloat4(&planenormalvec), -XMLoadFloat3(&m_parallelLightVec));
}



ComPtr<ID3D12Resource> Render::GetShadowTexture()
{	
	return GetTextureByPath("asset/img/alfatest.png");
}

void Render::SetSceneView()
{
	//���݂̃V�[��(�r���[�v���W�F�N�V����)���Z�b�g
	ID3D12DescriptorHeap* sceneheaps[] = { m_sceneDescHeap.Get() };
	m_cmdList->SetDescriptorHeaps(1, sceneheaps);
	m_cmdList->SetGraphicsRootDescriptorTable(0, m_sceneDescHeap->GetGPUDescriptorHandleForHeapStart());
}

Model* Render::GetLodedModel(int num)
{
	if (num >= m_modelTabel.size())
		return nullptr;
	else
		return m_modelTabel[num];
}

void Render::CameraUpdate()
{
	Mouse::Mouse_GetState();
	auto mousestate = Mouse::GetMouseState();
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	XMFLOAT4 planenormalvec(0, 1, 0, 0);

	auto result = m_swapchain->GetDesc1(&desc);
	if (FAILED(result))
		assert(0);

	if(Input::GetKeyTrigger('1'))
	{
		m_eye = XMFLOAT3(0, 15, -15);
	}
	else if (Input::GetKeyTrigger('2'))
	{
		m_eye = XMFLOAT3(0, 15, -55);
	}
	else if (Input::GetKeyTrigger('3'))
	{
		m_eye = XMFLOAT3(0, 15, 55);
	}
	else if (Input::GetKeyTrigger('4'))
	{
		m_eye = XMFLOAT3(-15, 15, 0);
		m_target = XMFLOAT3(0, 15, 15);
	}
	else if (Input::GetKeyTrigger('5'))
	{
		m_eye = XMFLOAT3(15, 15, 0);
		m_target = XMFLOAT3(0, 15, 15);
	}

	m_mappedSceneData->view = XMMatrixLookAtLH(XMLoadFloat3(&m_eye), XMLoadFloat3(&m_target), XMLoadFloat3(&m_up));
	m_mappedSceneData->proj = XMMatrixPerspectiveFovLH(XM_PIDIV4,//��p��45��
		static_cast<float>(desc.Width) / static_cast<float>(desc.Height),//�A�X��
		0.1f,//�߂���
		1000.0f//������
	);
	m_mappedSceneData->eye = m_eye;
	m_mappedSceneData->light = { m_parallelLightVec.x, m_parallelLightVec.y, m_parallelLightVec.z, 0 };
	m_mappedSceneData->shadow = XMMatrixShadow(XMLoadFloat4(&planenormalvec), -XMLoadFloat3(&m_parallelLightVec));
	m_oldMousePos = { mousestate->x, mousestate->y };
}

ComPtr<ID3D12DescriptorHeap> Render::CreateDescriptorHeapForImgui()
{
	ComPtr<ID3D12DescriptorHeap> ret;
	D3D12_DESCRIPTOR_HEAP_DESC decs = {};
	decs.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	decs.NodeMask = 0;
	decs.NumDescriptors = 1;
	decs.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	m_dev->CreateDescriptorHeap(&decs, IID_PPV_ARGS(ret.ReleaseAndGetAddressOf()));

	return ret;
}

Render::~Render()
{
	Uninit();
}

Render::Render() : m_parallelLightVec(1, -1, 1)
{
}


std::string helper::GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath)
{
	//�t�@�C���̃t�H���_��؂��\��/�̓��ނ��g�p�����\��������
	//�Ƃ�����������\��/�𓾂���΂����̂ŁA�o����rfind���Ƃ��r����
	//int�^�ɑ�����Ă���̂͌�����Ȃ������ꍇ��rfind��epos(-1��0xffffffff)��Ԃ�����
	int pathIndex1 = modelPath.rfind('/');
	int pathIndex2 = modelPath.rfind('\\');
	auto pathIndex = max(pathIndex1, pathIndex2);
	auto folderPath = modelPath.substr(0, pathIndex + 1);
	return folderPath + texPath;
}
std::wstring helper::GetTexturePathFromModelAndTexPath(const std::wstring& modelPath, const wchar_t* texPath)
{
	//�t�@�C���̃t�H���_��؂��\��/�̓��ނ��g�p�����\��������
	//�Ƃ�����������\��/�𓾂���΂����̂ŁA�o����rfind���Ƃ��r����
	//int�^�ɑ�����Ă���̂͌�����Ȃ������ꍇ��rfind��epos(-1��0xffffffff)��Ԃ�����
	int pathIndex1 = modelPath.rfind('/');
	int pathIndex2 = modelPath.rfind('\\');
	auto pathIndex = max(pathIndex1, pathIndex2);
	auto folderPath = modelPath.substr(0, pathIndex + 1);
	return folderPath + texPath;
}

///�t�@�C��������g���q���擾����
///@param path �Ώۂ̃p�X������
///@return �g���q
std::string helper::GetExtension(const std::string& path)
{
	int idx = path.rfind('.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

///�t�@�C��������g���q���擾����(���C�h������)
///@param path �Ώۂ̃p�X������
///@return �g���q
std::wstring helper::GetExtension(const std::wstring& path)
{
	int idx = path.rfind(L'.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

///�e�N�X�`���̃p�X���Z�p���[�^�����ŕ�������
///@param path �Ώۂ̃p�X������
///@param splitter ��؂蕶��
///@return �����O��̕�����y�A
pair<string, string> helper::SplitFileName(const std::string& path, const char splitter)
{
	int idx = path.find(splitter);
	pair<string, string> ret;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx + 1, path.length() - idx - 1);
	return ret;
}

pair<wstring, wstring> helper::SplitFileName(const std::wstring& path, const char splitter)
{
	int idx = path.find(splitter);
	pair<wstring, wstring> ret;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx + 1, path.length() - idx - 1);
	return ret;
}

///string(�}���`�o�C�g������)����wstring(���C�h������)�𓾂�
///@param str �}���`�o�C�g������
///@return �ϊ����ꂽ���C�h������
std::wstring helper::GetWideStringFromString(const std::string& str)
{
	//�Ăяo��1���(�����񐔂𓾂�)
	auto num1 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, nullptr, 0);

	std::wstring wstr;//string��wchar_t��
	wstr.resize(num1);//����ꂽ�����񐔂Ń��T�C�Y

	//�Ăяo��2���(�m�ۍς݂�wstr�ɕϊ���������R�s�[)
	auto num2 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, &wstr[0], num1);

	assert(num1 == num2);//�ꉞ�`�F�b�N
	return wstr;
}

std::string helper::GetStringFromWideString(const std::wstring& wstr)
{
	auto num1 = WideCharToMultiByte(CP_OEMCP, 0, wstr.c_str(), -1, (char*)NULL, 0, NULL, NULL);

	char* cpmutibyte = new CHAR[num1];

	auto num2 = WideCharToMultiByte(CP_OEMCP, 0, wstr.c_str(), -1, cpmutibyte, num1, NULL, NULL);

	assert(num1 == num2);

	std::string str(cpmutibyte, cpmutibyte + num1 - 1);
	delete[] cpmutibyte;
	return (str);
}

std::string helper::GetUTF8FromWideString(const std::wstring& wstr)
{
	auto num1 = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, (char*)NULL, 0, NULL, NULL);

	CHAR* cputf8 = new CHAR[num1];

	auto num2 = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, cputf8, num1, NULL, NULL);

	assert(num1 == num2);

	std::string str(cputf8, cputf8 + num1 - 1);
	delete[] cputf8;

	return (str);
}

bool helper::CheckResult(HRESULT& result, ID3DBlob* errBlob)
{
	if (FAILED(result)) {
#ifdef _DEBUG
		if (errBlob != nullptr) {
			std::string outmsg;
			outmsg.resize(errBlob->GetBufferSize());
			std::copy_n(static_cast<char*>(errBlob->GetBufferPointer()),
				errBlob->GetBufferSize(),
				outmsg.begin());
			OutputDebugString(outmsg.c_str());//�o�̓E�B���h�E�ɏo�͂��Ă�
		}
		assert(SUCCEEDED(result));
#endif
		return false;
	}
	else {
		return true;
	}
}
