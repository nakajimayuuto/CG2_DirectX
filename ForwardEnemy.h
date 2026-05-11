#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
#include "ForwardEnemyBasePhase.h"
class ForwardEnemy : public BaseEnemy {
public:
	void Initialize(Vector3 position) override;
	void Update() override;
	void Draw() override;

	//void ApproachPhaseUpdate();
	//void LeavePhaseUpdate();
	void Translate(Vector3 translate);

	Vector3 GetPosition() { return transform_.translate; };

	void ChangePhase(ForwardEnemyBasePhase* phase) { phase_ = phase; };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	static void (ForwardEnemy::* pFunc[])();
private:
	static inline float kLeaveSpeed = 0.1f;
	Vector3 velocity_;

	ForwardEnemyBasePhase* phase_ = nullptr;

	Transform transform_;
	Renderer::ModelBox model_;
};

