#include "main.h"
#include "App.h"

#ifdef _DEBUG
int main()
{
#else
#include<Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif

	auto app = App::Instance();

	if (FAILED(app->Init()))
	{
		return -1;
	}

	app->Update();
	app->Uninit();
}

