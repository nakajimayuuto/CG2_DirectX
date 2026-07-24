#pragma once
#include "Satlib.h"
#include "HPGauge.h"
#include <string>

/// <summary>
/// 自キャラ
/// </summary>
class Player : public Collider{
public:
	enum class Behavior {
		kRoot,
		kAttack,
		kDash,
		kJump,
		kDashAttack,
		kDashJumpAttack,
		kFall,
	};

	struct ConstAttack {
		float anticipationTime; // 振りかぶり時間.

		float chargeTime; // ため時間.

		float swingTime; // 攻撃時間.

		float recoveryTime; // 硬直時間.

		float anticipationSpeed; // 振りかぶり移動速度.

		float chargeSpeed; // ため移動速度.

		float swingSpeed; // 攻撃移動速度.
	};

	void Initialize();

	void Update();

	void Draw();

	bool GetIsMoving() { return isMoving_; };

	Transform* GetTransform() { return &transform_; };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	Vector3 GetWorldPosition() override { return transform_.GetAffineMatrix().GetMatrixToTranslate(); };

	bool GetIsAttack() { return isAttack_; };

	void OnCollision([[maybe_unused]] Collider* other)override;

	void TestWallClamp();
private:
	bool GetAttackButtonTrigger();
	bool GetJumpButtonTrigger();
	bool GetDownPress();

	void FloatingAccelerationChange();

	void BehaviorRootInitialize();
	void BehaviorRootUpdate();

	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate();
	void BehaviorAttackFinished();

	void BehaviorDashInitialize();
	void BehaviorDashUpdate();

	void BehaviorJumpInitialize();
	void BehaviorJumpUpdate();

	void BehaviorDashAttackInitialize();
	void BehaviorDashAttackUpdate();

	void BehaviorDashJumpAttackInitialize();
	void BehaviorDashJumpAttackUpdate();
	

	void BehaviorFallInitialize();
	void BehaviorFallUpdate();


	void InitializeFloatingGimmick();

	void UpdateFloatingGimmick();
private:
	bool isAttack_;
	bool isDash_;

	std::unique_ptr<HPGauge> hpGauge_;

	float currentHP_;
	float maxHP_;

	float damageCoolTimer_;
	float damageCoolTimeMax_;

	static inline float kSpeed = 10.0f;
	static inline float kSpeedDeceleration = 10.0f;
	static inline float kFloatingAcceleration = 20.0f;
	static inline float kDashSpeed = 25.0f;
	static inline float kCompletionRate = 0.25f;

	static inline uint16_t kFloatingAnimationPeriod = 120;

	static inline float kFloatingAmplitude = 0.2f;

	float floatingParameter = 0.0f;

	Behavior behavior_ = Behavior::kRoot;

	std::optional<Behavior> behaviorRequest_ = std::nullopt;

	static inline float kJumpFirstSpeed_ = 7.0f;
	static inline float kDashJumpFirstSpeed_ = 12.0f;

	Vector3 velocity_;
	static inline float kGravityAcceleration = 20.0f;

	bool isMoving_;

	float targetRotateY;
	Transform transformColliderOffset;
	Transform transformModel;

	Model model_;

	static inline float kTranslateBlankY = 1.2f;
	static inline float kBodyBlankY = 0.3f;

	static inline float kThreshold = 0.2f;
private:
	void SetNextAttackPhase(float timeMax);

	void AttackFirstInitialize();
	void AttackFirstUpdate();

	void AttackSecondInitialize();
	void AttackSecondUpdate();

	void AttackThreeInitialize();
	void AttackThreeUpdate();


private:
	Collider attackCollider_;
	uint32_t attackComboPhase_;
	uint32_t attackPhase_;
	float attackTimer_;
	float attackTimeMax_;
	bool useNextAttack_;

	Transform attackTransform_;


	static inline float kAttackFirstStart = 0.05f;
	static inline float kAttackFirstSpin = 0.2f;
	static inline float kAttackFirstFinish = 0.05f;

	static inline float kAttackFirstStartModelRotateX = Radian(20.0f);
	static inline float kAttackFirstSpinModelRotateX = Radian(340.0f);
	static inline float kAttackFirstSpinModelPosY = 0.7f;


	static inline float kAttackSecondStart = 0.05f;
	static inline float kAttackSecondSpin = 0.2f;
	static inline float kAttackSecondFinish = 0.05f;

	//static inline float kAttackFirstStartModelRotateX = Radian(20.0f);
	//static inline float kAttackFirstSpinModelRotateX = Radian(340.0f);
	//static inline float kAttackFirstSpinModelPosY = 0.7f;


	static inline float kAttackThirdStart = 0.1f;
	static inline float kAttackThirdSpin = 0.4f;
	static inline float kAttackThirdFinish = 0.1f;


	// DashAttack.
	static inline float kDashAttackStart = 0.3f;
};

