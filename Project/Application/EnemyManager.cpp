#include "EnemyManager.h"
EnemyManager* EnemyManager::GetInstance() {
	static EnemyManager instance;
	return &instance;
}

void EnemyManager::Initialize() {
	enemies_.clear();
}

void EnemyManager::Update() {
	for (auto& enemy : enemies_) {
		enemy->Update();
	}
}

void EnemyManager::Draw() {
	for (auto& enemy : enemies_) {
		enemy->Draw();
	}
}

void EnemyManager::SpawnEnemy(const Vector3 position){
	std::unique_ptr<IEnemy> enemy = std::make_unique<ShotEnemy>();
	enemy->Initialize();
	enemy->SetPosition(position);
	enemies_.push_back(std::move(enemy));
}
