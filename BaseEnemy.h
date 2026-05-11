#pragma once
#include "Satlib.h"
class BaseEnemy{
public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	bool GetIsAlive() { return isAlive_; };
protected:
	bool isAlive_ = false;
};

