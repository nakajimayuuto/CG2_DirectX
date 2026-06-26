#pragma once
#include "Satlib.h"
#include "BaseCharacter.h"

class Hammer : public BaseCharacter {
public:
	void Initialize();

	void Update();

	void Draw();

	void SetTargetTransform(Transform* transform) { transformTarget_ = transform; };

	bool GetIsFinished()const { return isFinished_; };

	Vector3 GetWorldPosition() override { return transform_.GetAffineMatrix().GetMatrixToTranslate(); }

	struct ConstAttack {
		float anticipationTime; // 振りかぶり時間.

		float chargeTime; // ため時間.

		float swingTime; // 攻撃時間.

		float recoveryTime; // 硬直時間.

		float anticipationSpeed; // 振りかぶり移動速度.

		float chargeSpeed; // ため移動速度.

		float swingSpeed; // 攻撃移動速度.
	};
private:
	float GetSumComboTime(uint32_t index);
private:
	struct AttackWork {
		float attackParameter = 0.0f;
		uint32_t comboIndex = 0;
		uint32_t inComboPhase = 0;
		bool comboNext = false;
	};

	Transform transform_;

	Transform* transformTarget_;

	bool isFinished_;

	Model model_;

	Vector3 beforeHammerRotate_ = { 0.0f,0.0f,0.0f };

	static inline float kStampAnimationMaxTime = 0.5f;

	static inline float kStartHammerRotateX = 0.0f;
	static inline float kStampHammerRotateX = Radian(90.0f);

	static inline Vector3 kRollingStartHammerRotate = { Radian(60.0f),0.0f,Radian(90.0f) };
	static inline float kRollingStartAnimationMaxTime = 0.25f;

	static inline Vector3 kRollingSwingHammerRotate = { Radian(480.0f),0.0f,Radian(90.0f) };
	static inline float kRollingSwingAnimationMaxTime = 0.25f;

	static inline Vector3 kExtraSwingHammerRotate = { Radian(1200.0f),0.0f,0.0f };

	static inline const uint32_t kComboNum = 3;

	AttackWork workAttack_;

	static inline std::array<Hammer::ConstAttack, Hammer::kComboNum> kConstAttacks_;

	float hammerAnimationTimer_ = 0.0f;
};

