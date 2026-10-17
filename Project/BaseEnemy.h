#pragma once
#include "Satlib.h"
class Player;
class GameScene;

class BaseEnemy : public Collider{
public:
	virtual void Initialize(const Vector3& position) = 0;

	virtual void Update() = 0;

	virtual void Draw() = 0;

	Vector3	GetWorldPosition();

	bool GetIsDead() { return isDead_; };

protected:
	bool isCollisionDisable_ = false;

	// 死亡判定.
	bool isDead_ = false;

	Model model_;

};

