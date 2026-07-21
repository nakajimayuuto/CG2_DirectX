#pragma once
#include "Satlib.h"

class GameCamera {
public:
	static GameCamera* GetInstance();

	void Initialize();

	void Update();

	void SetTarget(const Transform* target) { target_ = target; Reset(); };

	void SetEnemyTransform(const Transform* target) { targetEnemy_ = target; };

	void SetTargetIsMove(bool isMove) { isMove_ = isMove; };

	void Reset();
private:
	void FollowedUpdate();

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
	const Transform* targetEnemy_ = nullptr;

	bool isMove_;

	float preTargetRotateY_;

	float wallNearDirection_;

	float movingRadius_ = 70.0f;

	Vector3 interTarget_ = {};
	Vector3 interOffsetTarget_ = {};
};