#pragma once
#include "Object.h"
class Character : public Object
{
public:
	Character();
	Character(std::string modelpath);
	~Character();

	void Init()override;
	void Uninit()override;
	void Draw()override;

	void AutoRotation();
	void ResetRotate(){	m_rotation = { 0,0,0 };	}
	void SetMotion();
	bool GetMotionFlag();
	void MotionPlayAndStop();
	void SetModel(int num);
	void ResetMotion();
	class VMDmotion* GetMotion() { return m_motion; };

protected:
	void ActorUpdate()override;

private:
	class Model* m_model = nullptr;
	class VMDmotion* m_motion = nullptr;
	XMMATRIX m_martrix{};

	bool m_autoRotateFlag = false;
	static int m_modelNumber;

};

