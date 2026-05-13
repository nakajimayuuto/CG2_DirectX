#pragma once
#include "../Engine/Math/Collider.h"
#include <list>

class CollisionManager{
public:
	static CollisionManager* GetInstance();

	void ClearColliderList();

	void AddColliderList(Collider* collider) {colliders_.push_back(collider);};

	void CheckAllCollision();
private:
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);
private:
	std::list<Collider*> colliders_;
};

