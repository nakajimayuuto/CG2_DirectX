#pragma once
#include "Satlib.h"
#include "BaseCharacter.h"

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
		float anticipationTime;

		float chargeTime;

		float swingTime;

		float recoveryTime;

		float anticipationSpeed;

		float chargeSpeed;

		float swingSpeed;
	};

	void Initialize() override;

	void Update()override;

	void Draw()override;

	Transform* GetTransform() { return &transform_; };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
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

	static inline float kStampAnimationMaxTime = 0.5f;

	static inline float kStartHammerRotateX = 0.0f;
	static inline float kStampHammerRotateX = Radian(90.0f);

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



	bool isMoving_;

	float targetRotateY;

	Transform transformBody_;
	static inline Transform transformHead_;
	static inline Transform transformRArm_;
	static inline Transform transformLArm_;
	static inline Transform transformHammer_;

	static inline float kBodyBlankY = 1.2f;
};

