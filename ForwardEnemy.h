#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
class ForwardEnemy : public BaseEnemy {
public:
	enum class Phase {
		kApproach, // 接近する.
		kLeave, // 離脱する.
	};

	void Initialize(Vector3 position) override;
	void Update() override;
	void Draw() override;

	void ApproachPhaseUpdate();
	void LeavePhaseUpdate();

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	static void (ForwardEnemy::* pFunc[])();
private:
	static inline float kApproachSpeed = 0.1f;
	static inline float kLeaveSpeed = 0.1f;
	Vector3 velocity_;

	Phase phase_ = Phase::kApproach;

	Transform transform_;
	Renderer::ModelBox model_;
};

