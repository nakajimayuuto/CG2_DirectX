#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
class EnemyManager{
public:
	enum EnemyType {
		kDummy,
	};
	static EnemyManager* GetInstance();

	void Initialize();

	void Update();

	void Draw();

	void CreateEnemy(const Vector3 position, EnemyType type);
private:
	std::vector<std::unique_ptr<BaseEnemy>> enemies;
};

