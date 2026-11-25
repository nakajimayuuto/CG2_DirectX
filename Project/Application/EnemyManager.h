#pragma once
#include "IEnemy.h"
#include "ShotEnemy.h"
class EnemyManager{
public:
	static EnemyManager* GetInstance();

	void Initialize();

	void Update();

	void Draw();

	void SpawnEnemy(const Vector3 position);


private:
	std::vector<std::unique_ptr<IEnemy>> enemies_;
};

