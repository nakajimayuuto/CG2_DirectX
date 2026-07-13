#pragma once
#include "../Engine/Math/Collider.h"
#include <list>
enum CollisionAttributeName {
	kCollisionPlayer,
	kCollisionEnemy,
	kCollisionEnemyAttack,
};

class CollisionManager{
public:
	static CollisionManager* GetInstance();

	void Initialize();

	void ClearColliderList();

	void AddColliderList(Collider* collider) {
		colliders_.push_back(collider);
	};

	void CheckAllCollision();

	void DebugDraw();

	static void RegisterGlobalVariables();

	static void ApplyGlobalVariables();

	void CollisionAttributeInitialize() {
		for (uint32_t i = 0; i < kCollisionMaxNum; i++) {
			kCollisionAttributes[i] = 0b1 << i;
		}
	}

	uint32_t GetCollisionAttribute(CollisionAttributeName name) { return kCollisionAttributes[name]; };
private:
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);
private:
	static inline bool isColliderDraw_;

	std::list<Collider*> colliders_;

	static inline const uint32_t kCollisionMaxNum = 8;
	uint32_t kCollisionAttributes[kCollisionMaxNum];
};

