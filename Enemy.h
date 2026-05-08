#pragma once
#include "Satlib.h"
class Player;
class GameScene;

class Enemy{
public:
	void Initialize(const Vector3& position);

	void Update();

	void Draw();

	Vector3	GetWorldPosition();

	AABB GetAABB();

	bool GetIsDead() { return isDead_; };

	bool GetIsCollisionDisable() { return isCollisionDisable_; };

	void OnCollision(GameScene* scene,const Player* player);
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
	static inline const float kDeathAnimationParameterSpin = 1.0f;
	static inline const float kDeathAnimationParameterShrink = 0.5f;

	bool isCollisionDisable_ = false;

	// 移動.
	static inline const float kWalkSpeed = 0.01f;

	Vector3 velocity_ = {};

	// 当たり判定.
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// アニメーション.
	// 最初の角度.
	static inline const float kWalkMotionAngleStart = -15.0f;
	// 最後の角度.
	static inline const float kWalkMotionAngleEnd = 15.0f;
	// アニメーションの周期	(秒).
	static inline const float kWalkMotionTime = 1.0f;

	float walkTimer_ = 0.0f;



	// 死亡判定.
	bool isDead_ = false;

	Renderer::Model model_;

	Transform transform_;

};