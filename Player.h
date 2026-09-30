#pragma once
#include "Satlib.h"
#include "HPGauge.h"
#include <string>
#include "MapChipManager.h"

/// <summary>
/// 自キャラ
/// </summary>
class Player : public Collider {
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

	enum class LRDirection {
		kRight,
		kLeft,
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

	void SetStartPosition(const Vector3& position) { transform_.translate = position; transform_.translate.y = kTranslateBlankY; };

	void CircleWallClamp();

	void TutorialWallClamp();

	void SetIsImmune(bool isImmune) { isImmune_ = isImmune; };

	void SetTutorialMinClamp(float positionZ) { tutorialClampMinPosZ_ = positionZ; };
	void SetTutorialMaxClamp(float positionZ) { tutorialClampMaxPosZ_ = positionZ; };

	bool GetIsDeath() { return isDeath_; };
private:
	void SlashEffectCreate(Transform* targetTransform, uint32_t num);

	bool GetAttackButtonTrigger();
	bool GetJumpButtonTrigger();
	bool GetDownPress();
	float GetDirectionYPress();

	void FloatingAccelerationChange();

	void BehaviorRootInitialize();
	void BehaviorRootUpdate();

	void MovingUpdate();

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

	void MapChipDigUpdate();

	void BlockShotUpdate();

	void GravityUpdate();

	// 3 判定結果を反映して移動.

	void CollisionMoveUpdate(const CollisionMapInfo& info);

	// 4 天井に接触している場合の処理.
	void CellingCollisionUpdate(const CollisionMapInfo& info);

	// 5 壁に接触している場合の処理.
	void IsHitWallUpdate(const CollisionMapInfo& info);

	// 6 接地状態の切り替え.
	void IsGroundUpdate(const CollisionMapInfo& info);

	// 7 旋回制御.
	void TurningControl();

	void CheckFallVoid();

	void MapCollisionUpdate();
private:
	float knockbackParameter_ = 0.0f;
	static inline const float kKnockbackParameterBack = 0.2f;
	static inline const float kKnockbackParameterStop = 0.2f;

	static inline const float kAttenuationLanding = 0.01f;

	static inline const float kAttenuationWall = 0.1f;


	// 移動.
	static inline const float kAcceleration = 0.8f;
	static inline const float kAttenuation = 0.4f;
	static inline const float kJumpAttenuation = 0.01f;
	static inline const float kLimitRunSpeed = 20.0f;

	Transform transform_;
	Vector3 velocity_ = {};

	// ジャンプ.v
	static inline const float kLimitFallSpeed = 30.0f;
	static inline const float kJumpAcceleration = 20.0f;

	bool onGround_ = true;

	// 旋回制御.
	static inline const float kTimeTurn = 0.3f;

	LRDirection lrDirection_ = LRDirection::kRight;
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	std::string emitterName_ = "player_emitter";

	bool isAttack_;
	bool isDash_;
	bool isJump_;

	bool isImmune_;

	bool isDeath_;
	float deathTimer_;
	static inline float deathTimerMax = 1.0f;

	std::unique_ptr<HPGauge> hpGauge_;

	float currentHP_;
	float maxHP_;

	float damageCoolTimer_;
	float damageCoolTimeMax_;

	float movingRadius_ = 73.5f;

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

	static inline float kGravityAcceleration = 1.0f;

	bool isMoving_;

	float targetRotateY;
	Transform transformColliderOffset;
	Transform transformModel;

	Model model_;

	static inline float kTranslateBlankY = 1.2f;
	static inline float kBodyBlankY = 0.3f;

	static inline float kThreshold = 0.2f;
private:
	void CheckTutorialFlag();

	void CheckTutorialUpdate();
private:
	bool tutorialUsableMove_;
	bool tutorialUsableJump_;
	bool tutorialUsableDash_;
	bool tutorialUsableAttack_;

	float tutorialTimer_;

	float tutorialClampMinPosZ_;
	float tutorialClampMaxPosZ_;
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

