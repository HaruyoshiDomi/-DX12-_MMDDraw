#pragma once
class Component
{
public:
	Component(){}
	~Component(){}

	virtual void Init(){}
	virtual void Update(){}
	virtual void Uninit(){}
};

