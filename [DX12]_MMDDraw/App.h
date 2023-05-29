#pragma once

class App
{
public:
	static App* Instance();

	HRESULT Init();
	void Update();
	void Uninit();
	//float GetAPP ()FPSm_dwCurrentTime - m_dwExecLastTime
	~App();
private:

	App();
	App(const App&) = delete;
	void operator=(const App&) = delete;
	static App* m_instance;

	WNDCLASSEX m_windowclass = {};
	HWND m_hwnd = {};

	class Render* m_DXrender = nullptr;

	DWORD m_dwExecLastTime;
	DWORD m_dwCurrentTime;

	void CreateWindows(HWND& hwnd, WNDCLASSEX& windowClass);


protected:

	void DebugOutputFormatString(const char* format, ...)
	{
#ifdef _DEBUG
		va_list valist;
		va_start(valist, format);
		vprintf(format, valist);
		va_end(valist);
#endif
	}

	//アライメントにそろえたサイズを返す
	//@param size 元のサイズ
	//@param alignment　アライメントサイズ
	//@return アライメントをそろえたサイズ
	size_t AlignmentedSize(size_t size, size_t alignment)
	{
		return size + alignment - size % alignment;
	}

	void EnableDebugLayer()
	{
		ID3D12Debug* debuglayer = nullptr;
		auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debuglayer));
#ifdef _DEBUG
		if (result == S_OK)
			DebugOutputFormatString("DebugInterface OK \n");
		else
			DebugOutputFormatString("DebugInterface False \n");
#endif

		debuglayer->EnableDebugLayer();	//デバッグレイヤーを有効化
		debuglayer->Release();			//有効化したらインターフェースを開放する
	}

};

