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

	void SetMotion(class VMDmotion* motion);

protected:
	void ActorUpdate()override;

private:
	class Model* m_model = nullptr;
	class VMDmotion* m_motion = nullptr;
	XMMATRIX m_martrix{};

};

