#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
#include "ForwardEnemyBasePhase.h"
#include "BaseBullet.h"
#include <list>

class ForwardEnemy : public BaseEnemy {
public:
	~ForwardEnemy();
	void Initialize(Vector3 position) override;
	void Update() override;
	void Draw() override;

	void Fire();

	void Translate(Vector3 translate);

	Vector3 GetPosition() { return transform_.translate; };

	void ChangePhase(ForwardEnemyBasePhase* phase) {
		phase_ = phase; 
		phase_->Initialize();
	};

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	//static void (ForwardEnemy::* pFunc[])();
private:
	static inline float kBulletSpeed = 1.0f;

	Vector3 velocity_;

	ForwardEnemyBasePhase* phase_ = nullptr;

	Transform transform_;
	Renderer::ModelBox model_;

	std::list<BaseBullet*> bullets_;
};

