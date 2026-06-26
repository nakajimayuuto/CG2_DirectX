#pragma once
#include "Satlib.h"

class LockOn;

class FollowCamera{
public:
	void Initialize();

	void Update();

	void SetTarget(const Transform* target) { target_ = target; Reset(); };

	void SetLockOn(const LockOn* lockOn) { lockOn_ = lockOn; };

	void Reset();
private:
	Vector3 GetOffset()const;
private:
	static inline float kCompletionRate = 0.25f;

	static inline Vector3 kOffset = { 0.0f,2.0f,-20.0f };

	static inline float kRotateSpeed = Radian(1.0f);

	static inline float kMouseRotateSpeed = Radian(0.1f);

	float destinationAngleY_;
	
	Transform transform_;

	const Transform* target_ = nullptr;

	Vector3 interTarget_ = {};

	const LockOn* lockOn_ = nullptr;
};