#include "main.h"
#include "Character.h"
#include "PMDmodel.h"
#include "PMXmodel.h"
#include "Render.h"
#include "input.h"
#include "Manager.h"

std::vector<Object*> obj = {};
void Manager::Init()
{
	Input::Init();
	obj.push_back(new Character());
	int num = 0;
	for (auto m : obj )
	{
		m->Init();
		m->SetPosition(XMFLOAT3(num * 10, 0, 0));
		num++;
	}

}

void Manager::Update()
{

	Input::Update();
	Render::Instance()->CameraUpdate();
	for (auto m : obj)
	{
		m->Update();
	}
}

void Manager::Draw()
{
	for (auto m : obj)
	{
		m->Draw();
	}

}

void Manager::Uninit()
{
	Input::Uninit();
	for (auto m : obj)
	{
		delete m;
	}
}