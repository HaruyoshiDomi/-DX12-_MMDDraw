#include "main.h"
#include "Render.h"
#include "VMDmotion.h"
#include "PMDmodel.h"
#include "PMXmodel.h"
#include "input.h"
#include "Character.h"

int Character::m_modelNumber = 0;
Character::Character()
{
	//m_motion = new VMDmotion("asset/motion/JUMP UP.vmd");
	//m_model = new PMDmodel("asset/Model/PMD/結月ゆかり_純_ver1.0/結月ゆかり_純_ver1.0.pmd", *Render::Instance());
	auto render = Render::Instance();
	m_model = render->GetLodedModel();
	//m_model->SetMotion(m_motion);
}

Character::Character(std::string modelpath)
{
	if(helper::GetExtension(modelpath) == "pmx")
		m_model = new PMXmodel(modelpath.c_str(), *Render::Instance());
	else if(helper::GetExtension(modelpath) == "pmd")
		m_model = new PMDmodel(modelpath.c_str(), *Render::Instance());

}

Character::~Character()
{
	delete m_model;
	if(m_motion)
		delete m_motion;
}

void Character::Init()
{
	m_position = { 0,0,0 };
	m_rotation = { 0,0,0 };
	m_scal = { 1,1,1 };
}

void Character::Uninit()
{
	delete m_model;
}

void Character::Draw()
{
	m_model->Draw();
}

void Character::ActorUpdate()
{
	static int mxnum = Render::Instance()->GetCharacterNum() - 1;
	if (!m_motion)
	{
		if(m_autoRotateFlag)
			m_rotation.y += 0.01;
		if (Input::GetKeyTrigger('M'))
		{
			m_motion = new VMDmotion("asset/motion/JUMP UP.vmd");
			for (int i = 0; i < Render::Instance()->GetCharacterNum(); i++)
			{
				Render::Instance()->GetLodedModel(i)->SetMotion(m_motion);
			}
			m_model->SetMotion(m_motion);
			m_rotation.y = 0;
			mxnum = 1;
		}
	}
	else
	{
		if (Input::GetKeyTrigger(VK_SPACE))
			if (m_motion->GetMotionFlag())
				m_motion->SetMotionFlag(false);
			else
				m_motion->SetMotionFlag(true);
	}

	if (Input::GetKeyTrigger('C'))
	{
		++m_modelNumber;
		if (m_modelNumber > mxnum)
			m_modelNumber = 0;

		m_model = Render::Instance()->GetLodedModel(m_modelNumber);
		if(m_motion && m_modelNumber <= mxnum  )
			m_model->SetMotion(m_motion);

	}

	m_martrix = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
	m_martrix *= XMMatrixScaling(m_scal.x, m_scal.y, m_scal.z);
	m_martrix *= XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	m_model->Update(m_martrix);
}


void Character::AutoRotation()
{
	if (!m_autoRotateFlag)
		m_autoRotateFlag = true;
	else
		m_autoRotateFlag = false;
}

void Character::SetMotion()
{
	m_motion = new VMDmotion("asset/motion/JUMP UP.vmd");
	for (int i = 0; i < Render::Instance()->GetCharacterNum(); i++)
	{
		Render::Instance()->GetLodedModel(i)->SetMotion(m_motion);
	}
	m_model->SetMotion(m_motion);
	m_rotation.y = 0;
}

bool Character::GetMotionFlag()
{
	if (m_motion)
		return true;
	else
		return false;
}

void Character::MotionPlayAndStop()
{
	if (m_motion->GetMotionFlag())
		m_motion->SetMotionFlag(false);
	else
		m_motion->SetMotionFlag(true);
}

void Character::SetModel(int num)
{
	m_modelNumber = num;
	m_model = Render::Instance()->GetLodedModel(m_modelNumber);
	if (m_motion && m_modelNumber <= (Render::Instance()->GetCharacterNum() - 1))
		m_model->SetMotion(m_motion);
}

void Character::ResetMotion()
{
	if (m_motion)
		m_motion->ResetMotion();
}
