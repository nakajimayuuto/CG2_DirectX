#pragma once
#include "Satlib.h"
class FollowCamera{
public:
	void Initialize();

	void Update();

	void SetTarget(const Transform* target) { target_ = target; };
private:
	static inline Vector3 kOffset = { 0.0f,2.0f,-20.0f };

	static inline float kRotateSpeed = Radian(1.0f);

	static inline float kMouseRotateSpeed = Radian(0.1f);

	Transform transform_;

	const Transform* target_ = nullptr;
};