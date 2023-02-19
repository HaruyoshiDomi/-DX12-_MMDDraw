#include "main.h"
#include "PMDmodel.h"
#include "PMXmodel.h"
#include "Render.h"
#include "input.h"
#include"imgui\imgui.h"
#include"imgui\imgui_impl_win32.h"
#include"imgui\imgui_impl_dx12.h"
#include "Manager.h"

Manager* Manager::m_instance = nullptr;

void Manager::Init()
{
	Input::Init();
	AddObject<Character>();
	int num = 0;
	for (auto m : m_obj)
	{
		m->Init();
		m->SetPosition(XMFLOAT3(num * 10, 0, 0));
		num++;
	}

}

void Manager::Update()
{

	Input::Update();
	for (auto m : m_obj)
	{
		m->Update();
	}

}

void Manager::Draw()
{
	for (auto m : m_obj)
	{
		m->Draw();
	}
	float m = 0;

}

void Manager::Uninit()
{
	Input::Uninit();
	for (auto m : m_obj)
	{
		delete m;
	}
}

Manager::Manager()
{
}
