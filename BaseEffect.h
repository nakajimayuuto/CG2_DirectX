#pragma once
#include "Satlib.h"
#include <array>

class BaseEffect {
public:
	enum class EffectType {
		kHit,
		kGuard,
	};

	static BaseEffect* Create(Vector3 position,EffectType type);

	virtual void Initialize(Vector3 position) = 0;

	virtual void Update() = 0;

	virtual void Draw() = 0;

	bool GetIsDelete() { return isDelete_; };
protected:
	bool isDelete_ = false;
};

