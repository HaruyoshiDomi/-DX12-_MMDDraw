#include "main.h"
#include "Render.h"
#include "PMDmodel.h"
#include <thread>
#include "Mouse.h"
#include "App.h"

App* App::m_instance = nullptr;

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//�E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);//OS�ɑ΂��āu�������̃A�v���͏I���v�Ɠ`����
		return 0;
	}

	switch (msg)
	{
	case WM_ACTIVATEAPP:
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::Mouse_ProcessMessage(msg, wparam, lparam);
		break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam); //����̏������s��
}


App* App::Instance()
{
	if (!m_instance)
		m_instance = new App();
	return m_instance;
}

HRESULT App::Init()
{
	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

	//�t���[���J�E���g������
	timeBeginPeriod(1);
	m_dwExecLastTime = timeGetTime();
	m_dwCurrentTime = 0;

	CreateWindows(m_hwnd, m_windowclass);
#ifdef _DEBUG
	EnableDebugLayer();
#endif
	m_DXrender = Render::Instance();

	if (FAILED(m_DXrender->Init(m_hwnd)))
		return S_FALSE;

	Mouse::Mouse_Initialize(m_hwnd);

	return S_OK;
}



void App::Update()
{

	ShowWindow(m_hwnd, SW_SHOW);

	MSG msg = {};

	while (msg.message != WM_QUIT)
	{

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{

			m_dwCurrentTime = timeGetTime();

			if ((m_dwCurrentTime - m_dwExecLastTime) >= (1000 / FPS))
			{
				m_dwExecLastTime = m_dwCurrentTime;

			}
			m_DXrender->Update();
			m_DXrender->Draw();
		}
	}

}

void App::Uninit()
{
	timeEndPeriod(1);
	UnregisterClass(m_windowclass.lpszClassName, m_windowclass.hInstance);
	delete m_instance;
	m_instance = nullptr;
}

App::~App()
{
}

App::App()
{
}

void App::CreateWindows(HWND& hwnd, WNDCLASSEX& windowClass)
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpfnWndProc = (WNDPROC)WindowProcedure; //�R�[���o�b�N�֐��̎w��
	windowClass.lpszClassName = ("DirectX12");		  //�A�v���P�[�V�����N���X��
	windowClass.hInstance = GetModuleHandle(nullptr);   //�n���h���̏���
	RegisterClassEx(&windowClass);	//�A�v���P�[�V�����N���X�i�E�B���h�E�N���X�̎w���OS�ɓ`����j

	RECT wrc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT }; //�E�B���h�E�T�C�Y�����߂�
		//�֐���ǉ����ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//�E�B���h�E�I�u�W�F�N�g�̐���
	hwnd = CreateWindow(
		windowClass.lpszClassName,		//�N���X���̎w��
		("�uDX12�vMMDDraw"),			//�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,	//�^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,			//�\��x���W��OS�ɂ��C��
		CW_USEDEFAULT,			//�\��y���W��OS�ɂ��C��
		wrc.right - wrc.left,	//�E�B���h�E��
		wrc.bottom - wrc.top,	//�E�B���h�E��
		nullptr,				//�e�E�B���h�E�n���h��
		nullptr,				//���j���[�n���h��
		windowClass.hInstance,			//�Ăяo���A�v���P�[�V�����n���h��
		nullptr					//�ǉ��p�����[�^
	);

}