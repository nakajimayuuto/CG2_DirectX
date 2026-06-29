#pragma once
#include "Satlib.h"
#include "BaseCharacter.h"
#include "Hammer.h"

class LockOn;

/// <summary>
/// 自キャラ
/// </summary>
class Player : public BaseCharacter{
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

	void Initialize() override;

	void Update()override;

	void Draw()override;

	Transform* GetTransform() { return &transform_; };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	void SetLockOn(LockOn* lockOn) { lockOn_ = lockOn; };

	Vector3 GetWorldPosition() override { return transform_.GetAffineMatrix().GetMatrixToTranslate(); };

	Collider* GetHammerCollider() { return hammerOfJustice_->GetHammerStampCollision(); };

	void OnCollision([[maybe_unused]] Collider* other)override;
private:
	float GetSumComboTime(uint32_t index);

	void BehaviorRootInitialize();
	void BehaviorRootUpdate();

	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate();

	void BehaviorDashInitialize();
	void BehaviorDashUpdate();

	void BehaviorJumpInitialize();
	void BehaviorJumpUpdate();


	void InitializeFloatingGimmick();

	void UpdateFloatingGimmick();
private:
	enum AttackPhase {
		kCharge,
		kStamp,
		kStay,
	};

	struct WorkDash {
		float dashParameter_ = 0.0f;
	};

	struct AttackWork {
		float attackParameter = 0.0f;
		uint32_t comboIndex = 0;
		uint32_t inComboPhase = 0;
		bool comboNext = false;
	};

	static inline float kSpeed = 0.3f;
	static inline float kCompletionRate = 0.25f;

	static inline uint16_t kFloatingAnimationPeriod = 120;

	static inline float kFloatingAmplitude = 0.3f;

	float floatingParameter = 0.0f;

	Behavior behavior_ = Behavior::kRoot;

	std::optional<Behavior> behaviorRequest_ = std::nullopt;

	Vector3 beforeHammerRotate_ = {0.0f,0.0f,0.0f};

	static inline float kStampAnimationMaxTime = 0.5f;

	static inline float kStartHammerRotateX = 0.0f;
	static inline float kStampHammerRotateX = Radian(90.0f);

	static inline Vector3 kRollingStartHammerRotate = { Radian(60.0f),0.0f,Radian(90.0f) };
	static inline float kRollingStartAnimationMaxTime = 0.25f;

	static inline Vector3 kRollingSwingHammerRotate = { Radian(480.0f),0.0f,Radian(90.0f) };
	static inline float kRollingSwingAnimationMaxTime = 0.25f;

	static inline Vector3 kExtraSwingHammerRotate = {Radian(1200.0f),0.0f,0.0f};

	AttackPhase attackPhase_;

	static inline const uint32_t kComboNum = 3;

	AttackWork workAttack_;

	static inline std::array<Player::ConstAttack, Player::kComboNum> kConstAttacks_;

	float hammerAnimationTimer_ = 0.0f;

	static inline float kBehaviorDashTime = 0.5f;

	WorkDash workDash_;

	static inline float kJumpFirstSpeed_ = 1.0f;

	Vector3 velocity_;
	static inline float kGravityAcceleration = 0.05f;

	std::unique_ptr<Hammer> hammerOfJustice_ = nullptr;

	std::unique_ptr<Particles> particles_ = nullptr;

	std::unique_ptr<Emitter> emitter_ = nullptr;

	bool isMoving_;

	float targetRotateY;

	Transform transformBody_;
	static inline Transform transformHead_;
	static inline Transform transformRArm_;
	static inline Transform transformLArm_;
	static inline Transform transformHammer_;

	static inline float kBodyBlankY = 1.2f;

	static inline float kThreshold = 0.2f;

	LockOn* lockOn_;
};

