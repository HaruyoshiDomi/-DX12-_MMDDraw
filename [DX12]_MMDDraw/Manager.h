#pragma once
#include <vector>
#include <typeinfo>
#include "Character.h"
class Manager
{
public:
	void Init();
	void Update();
	void Draw();
	void Uninit();

	static Manager* GetInstance()
	{
		if (!m_instance)
			m_instance = new Manager();

		return m_instance;
	}

	~Manager(){}
private:
	Manager();
	Manager(const Manager&) = delete;
	void operator=(const Manager&) = delete;
	static Manager* m_instance;
	std::vector<class Object*> m_obj;


public:
	template <typename T>
	T* AddObject()
	{
		T* obj = new T();
		m_obj.push_back(obj);

		return obj;
	}

	template <typename T>
	std::vector<T*> GetObjects()
	{
		std::vector<T*> objects;//STL‚Ì”z—ñ
		for (Object* object : m_obj)
		{
			if (typeid(*object) == typeid(T))
			{
				objects.push_back((T*)object);
			}
		}
		return objects;
	}

};


