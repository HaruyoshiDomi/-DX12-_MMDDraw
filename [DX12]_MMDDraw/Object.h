#pragma once
#include "main.h"
#include "Component.h"
class Object
{
public:
	Object() {}
	~Object() {}

	virtual void Init() = 0;
	virtual void Uninit() = 0;
	virtual void Draw() = 0;
	virtual void Update() 
	{
		ComponentUpdate();
		ActorUpdate();
	}


	void SetPosition(XMFLOAT3 pos) { m_position = pos; }

private:

	std::list<Component> m_childes;

	void ComponentUpdate()
	{
		for (auto c : m_childes)
		{
			c.Update();
		}
	}

protected:

	XMFLOAT3 m_position{};
	XMFLOAT3 m_rotation{};
	XMFLOAT3 m_scal{};

	virtual void ActorUpdate() = 0;

};

