#pragma once
#include "Satlib.h"
class Player;
class GameScene;

class BaseEnemy {
public:
	virtual void Initialize(const Vector3& position) = 0;

	virtual void Update() = 0;

	virtual void Draw() = 0;

	virtual void OnCollision(GameScene* scene, Player* player) = 0;

	virtual AABB GetAABB() = 0;

	Vector3	GetWorldPosition();

	bool GetIsDead() { return isDead_; };

	bool GetIsCollisionDisable() { return isCollisionDisable_; };

protected:
	bool isCollisionDisable_ = false;

	// 死亡判定.
	bool isDead_ = false;

	Renderer::Model model_;

	Transform transform_;

};

