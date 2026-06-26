#pragma once
#include "../Engine/Math/Collider.h"
#include <list>
class CollisionManager{
public:
	static CollisionManager* GetInstance();

	void Initialize();

	void ClearColliderList();

	void AddColliderList(Collider* collider) {colliders_.push_back(collider);};

	void CheckAllCollision();

	void DebugDraw();

	static void RegisterGlobalVariables();

	static void ApplyGlobalVariables();
private:
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);
private:
	static inline bool isColliderDraw_;

	std::list<Collider*> colliders_;
};

