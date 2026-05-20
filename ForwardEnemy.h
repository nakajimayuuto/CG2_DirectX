#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
#include "ForwardEnemyBasePhase.h"
#include "BaseBullet.h"
#include "./Scene//IScene.h"
#include "HomingBullet.h"
#include "NormalBullet.h"
#include <list>

class ForwardEnemy : public BaseEnemy {
public:
	ForwardEnemy() = default;
	~ForwardEnemy() override;
	void Initialize(Vector3 position) override;
	void Update() override;
	void Draw() override;

	void OnCollision() override;

	void Fire(BaseBullet* bullet);

	void Translate(Vector3 translate);

	Vector3 GetPosition() { return transform_.translate; };

	void SetPhase(std::unique_ptr<ForwardEnemyBasePhase> phase) { phase_ = std::move(phase);};

	void ChangePhase(std::unique_ptr<ForwardEnemyBasePhase> phase) {
		phase_ = std::move(phase);
		phase_->Initialize(this);
	};

	void SetGameScene(std::unique_ptr<IScene> gameScene) { gameScene_ = std::move(gameScene); };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	//static void (ForwardEnemy::* pFunc[])();
private:
	static inline float kBulletSpeed = 0.5f;

	Vector3 velocity_;

	std::unique_ptr<IScene> gameScene_ = nullptr;

	std::unique_ptr<ForwardEnemyBasePhase> phase_ = nullptr;
};

