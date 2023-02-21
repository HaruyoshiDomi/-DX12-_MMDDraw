#include "main.h"
#include "Render.h"
#include "PMDmodel.h"
#include <thread>
#include "Mouse.h"
#include"imgui\imgui.h"
#include"imgui\imgui_impl_win32.h"
#include"imgui\imgui_impl_dx12.h"
#include"imgui\imgui_ja_gryph_ranges.h"
#include "App.h"

App* App::m_instance = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);//OSに対して「もうこのアプリは終わる」と伝える
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
	ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);

	return DefWindowProc(hwnd, msg, wparam, lparam); //既定の処理を行う
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

	//フレームカウント初期化
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

	if(!ImGui::CreateContext())
		return S_FALSE;

	bool blnresult = ImGui_ImplWin32_Init(m_hwnd);
	if(!blnresult)
		return S_FALSE;

	blnresult = ImGui_ImplDX12_Init(
		m_DXrender->Device().Get(),												//dx12デバイス
		3,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_DXrender->GetHeapForImgui().Get(),
		m_DXrender->GetHeapForImgui()->GetCPUDescriptorHandleForHeapStart(),
		m_DXrender->GetHeapForImgui()->GetGPUDescriptorHandleForHeapStart()
	);
	auto& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("imgui/fonts/Migu 1M Regular.ttf", 15.0f, nullptr, glyphRangesJapanese);
	Mouse::Mouse_Initialize(m_hwnd);

	return S_OK;
}



void App::Update()
{

	ShowWindow(m_hwnd, SW_SHOW);
	UpdateWindow(m_hwnd);
	MSG msg = {};
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

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
	windowClass.lpfnWndProc = (WNDPROC)WindowProcedure; //コールバック関数の指定
	windowClass.lpszClassName = ("DirectX12");		  //アプリケーションクラス名
	windowClass.hInstance = GetModuleHandle(nullptr);   //ハンドルの所得
	RegisterClassEx(&windowClass);	//アプリケーションクラス（ウィンドウクラスの指定をOSに伝える）

	RECT wrc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT }; //ウィンドウサイズを決める
		//関数を追加ってウィンドウのサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウオブジェクトの生成
	hwnd = CreateWindow(
		windowClass.lpszClassName,		//クラス名の指定
		("「DX12」MMDDraw"),			//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	//タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT,			//表示x座標はOSにお任せ
		CW_USEDEFAULT,			//表示y座標はOSにお任せ
		wrc.right - wrc.left,	//ウィンドウ幅
		wrc.bottom - wrc.top,	//ウィンドウ高
		nullptr,				//親ウィンドウハンドル
		nullptr,				//メニューハンドル
		windowClass.hInstance,			//呼び出しアプリケーションハンドル
		nullptr					//追加パラメータ
	);

}