#pragma once
#include "Satlib.h"

class FollowCamera{
public:
	static FollowCamera* GetInstance();

	void Initialize();

	void Update();

	void SetTarget(const Transform* target) { target_ = target; Reset(); };

	void SetTargetIsMove(bool isMove) { isMove_ = isMove; };

	void Reset();
private:
	Vector3 GetOffset()const;
private:
	static inline float kCompletionRate = 0.25f;

	static inline float kAutoCompletionRate = 0.025f;

	static inline Vector3 kOffset = { 0.0f,2.0f,-20.0f };

	static inline float kRotateSpeed = Radian(1.0f);

	static inline float kMouseRotateSpeed = Radian(0.1f);

	float destinationAngleY_;
	float autoAngleY_;
	
	static inline float kLerpPlayerDirectionMin_ = 30.0f;
	static inline float kLerpPlayerDirectionMax_ = 90.0f;
	static inline float kLerpPlayerDirectionEase_ = 175.0f;

	float angleDirection_;

	Transform transform_;

	const Transform* target_ = nullptr;

	 bool isMove_;

	Vector3 interTarget_ = {};
};