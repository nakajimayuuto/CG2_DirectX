#include "EnemyManager.h"
#include "Dummy.h"

EnemyManager* EnemyManager::GetInstance() {
	static EnemyManager instance;
	return &instance;
}

void EnemyManager::Initialize() {
	enemies.clear();
}

void EnemyManager::Update() {
	for (auto&  enemy : enemies) {
		enemy->Update();
		CollisionManager::GetInstance()->AddColliderList(enemy.get());
	}
}

void EnemyManager::Draw() {
	for (auto&  enemy : enemies) {
		enemy->Draw();
	}
}

void EnemyManager::CreateEnemy(const Vector3 position,EnemyType type){
	std::unique_ptr<BaseEnemy> newEnemy;
	switch (type){
	case EnemyManager::kDummy:
		newEnemy = std::make_unique<Dummy>();
		break;
	default:
		break;
	}

	newEnemy->Initialize(position);
	enemies.push_back(std::move(newEnemy));
}
