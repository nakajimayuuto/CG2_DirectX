#pragma once
#include "../Satlib.h"
#include "IScene.h"

#include "../Player.h"
#include "../Skydome.h"
#include "../BaseEnemy.h"
#include "../RailCameraController.h"
#include "../ForwardEnemyBasePhase.h"
#include "../LockOn.h"
#include <sstream>

class GameScene : public IScene{
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void RegisterGlobalVariables();
	void ApplyGlobalVariables();

	void AddBullet(BaseBullet* baseBullet);
private:
	void SpawnEnemy(const Vector3& position, ForwardEnemyBasePhase* type);

	void BulletRemoveCheck();
	void EnemyRemoveCheck();

	void CheckAllCollision();

	void CheckCollisionPair(Collider* colliderA,Collider* colliderB);
	
	void UpdateEnemyPopCommands();

	void LoadEnemyPopData();
private:
	std::shared_ptr<Player> player_;

	std::list<std::shared_ptr<BaseEnemy>> enemies_;

	Skydome* skydome_ = nullptr;

	RailCameraController* railCameraController_ = nullptr;

	LockOn* lockOn_ = nullptr;

	Renderer::Model groundModel_;

	std::list<BaseBullet*> bullets_;

	// 発生制御スプリクト.
	std::stringstream enemyPopCommands;

	bool isWait_ = false;
	int32_t waitTimer_ = 0;
};

