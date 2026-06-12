#pragma once
#include "Satlib.h"
class BaseCharacter{
public:
	virtual void Initialize();

	virtual void Update();

	virtual void Draw();
protected:
	std::map<std::string, Model> models_;
	Transform transform_;
};

