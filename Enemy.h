#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
class Player;
class GameScene;

class Enemy final : public BaseEnemy{
public:
	void Initialize(const Vector3& position) override;

	void Update() override;

	void Draw() override;

	void OnCollision(GameScene* scene, Player* player) override;

	AABB GetAABB() override;

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	enum class Behavior {
		kUnknown, // リクエスト無し.
		kRoot, // 通常状態.
		kDeathAnimation, // 攻撃中.
	};

	void WalkAnimationUpdate();

	// 【Behavior】
	// Root.

	void BehaviorRootInitialize();
	void BehaviorRootUpdate();

	//DeathAnimation.
	void BehaviorDeathAnimationInitialize();
	void BehaviorDeathAnimationUpdate();
private:
	/*===========================================================
	Behavior.
	===========================================================*/
	Behavior behavior_ = Behavior::kRoot;
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// DeathAnimation.

	enum class DeathAnimationPhase {
		kSpin, // 回転.
		kShrink, // 収縮.
		kDeath, // フラグ変更.
	};

	DeathAnimationPhase deathAnimationPhase_;

	float deathAnimationParameter_ = 0.0f;
	static inline float kDeathAnimationParameterSpin = 1.0f;
	static inline float kDeathAnimationParameterShrink = 0.5f;

	// 移動.
	static inline float kWalkSpeed = 0.01f;

	Vector3 velocity_ = {};

	// 当たり判定.
	static inline float kWidth = 0.8f;
	static inline float kHeight = 0.8f;

	// アニメーション.
	// 最初の角度.
	static inline float kWalkMotionAngleStart = -15.0f;
	// 最後の角度.
	static inline float kWalkMotionAngleEnd = 15.0f;
	// アニメーションの周期	(秒).
	static inline float kWalkMotionTime = 1.0f;

	float walkTimer_ = 0.0f;

};