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
	void SetTargetIsDash(bool isDash) { isDash_ = isDash; };

	void Reset();
private:
	void FollowedUpdate();

	void FollowedControlAction();

	void FollowedWallClamp();

	void FollowedDash();

	void FollowedTarget();

	Vector3 GetOffset()const;

	//void IncrementEaseTimer();
private:
	static inline float kNormalFovY = 0.45f;
	static inline float kDashFovY = 0.60f;

	static inline float kCompletionRate = 0.25f;

	static inline float kAutoCompletionRate = 0.025f;

	static inline Vector3 kOffset = { 0.0f,2.0f,-20.0f };

	static inline float kRotateSpeed = Radian(1.0f);

	static inline float kMouseRotateSpeed = Radian(0.1f);

	float destinationPlayerAngleY_;
	float destinationDashAngleY_;
	float destinationTargetAngleY_;
	float destinationAngleY_;
	float autoAngleY_;

	float dashEaseTimer_;
	float targetEaseTimer_;

	static inline float kLerpPlayerDirectionMin_ = 30.0f;
	static inline float kLerpPlayerDirectionMax_ = 90.0f;
	static inline float kLerpPlayerDirectionEase_ = 175.0f;

	float angleDirection_;

	Transform transform_;

	const Transform* target_ = nullptr;
	const Transform* targetEnemy_ = nullptr;

	bool isMove_;
	bool isDash_;

	float preTargetRotateY_;

	float wallNearDirection_;

	float movingRadius_ = 75.0f;

	float distanceToCenter_;

	Vector3 interTarget_ = {};
	Vector3 interOffsetTarget_ = {};
};