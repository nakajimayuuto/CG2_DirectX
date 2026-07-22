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

	void OnCollision([[maybe_unused]] Collider* other)override;
private:
	void BehaviorRootInitialize();
	void BehaviorRootUpdate();

	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate();
	void BehaviorAttackFinished();

	void BehaviorDashInitialize();
	void BehaviorDashUpdate();

	void BehaviorJumpInitialize();
	void BehaviorJumpUpdate();


	void InitializeFloatingGimmick();

	void UpdateFloatingGimmick();
private:
	std::unique_ptr<HPGauge> hpGauge_;

	float currentHP_;
	float maxHP_;

	float damageCoolTimer_;
	float damageCoolTimeMax_;

	static inline float kSpeed = 10.0f;
	static inline float kDashSpeed = 25.0f;
	static inline float kCompletionRate = 0.25f;

	static inline uint16_t kFloatingAnimationPeriod = 120;

	static inline float kFloatingAmplitude = 0.2f;

	float floatingParameter = 0.0f;

	Behavior behavior_ = Behavior::kRoot;

	std::optional<Behavior> behaviorRequest_ = std::nullopt;

	static inline float kJumpFirstSpeed_ = 10.0f;

	Vector3 velocity_;
	static inline float kGravityAcceleration = 0.5f;

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

	Transform attackTransform_;


	static inline float kAttackFirstStart = 0.05f;
	static inline float kAttackFirstSpin = 0.2f;
	static inline float kAttackFirstFinish = 0.05f;
	static inline float kAttackSecondStart = 0.3f;

	static inline float kAttackFirstStartModelRotateX = Radian(20.0f);
	static inline float kAttackFirstSpinModelRotateX = Radian(340.0f);
	static inline float kAttackFirstSpinModelPosY = 0.7f;
};

