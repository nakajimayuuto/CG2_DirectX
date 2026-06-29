#include "CollisionManager.h"
#include "../Engine/Renderer/Renderer.h"
#include "../Engine/SystemFile/GlobalVariables.h"
CollisionManager* CollisionManager::GetInstance() {
	static CollisionManager instance;
	return &instance;
}

void CollisionManager::ClearColliderList(){
	colliders_.clear();
}

void CollisionManager::CheckAllCollision() {
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); itrA++) {
		Collider* colliderA = *itrA;
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;
		for (; itrB != colliders_.end(); itrB++) {
			Collider* colliderB = *itrB;

			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {
	if (
		((colliderA->GetCollisionAttribute() & colliderB->GetCollisionMask()) == 0x0) ||
		((colliderB->GetCollisionAttribute() & colliderA->GetCollisionMask()) == 0x0)
		) {
		return;
	}

	Sphere sphereA;
	Sphere sphereB;
	sphereA.center = colliderA->GetWorldPosition();
	sphereA.radius = colliderA->GetRadius();

	sphereB.center = colliderB->GetWorldPosition();
	sphereB.radius = colliderB->GetRadius();

	if (Collision::SphereToSphere(sphereA, sphereB)) {
		colliderA->OnCollision(colliderB);
		colliderB->OnCollision(colliderA);
	}
}

void CollisionManager::DebugDraw() {
	if (!isColliderDraw_) {
		return;
	}

	for (Collider* collider : colliders_) {
		Transform transform = Transform::GetInitialValue({collider->GetRadius(),collider->GetRadius(),collider->GetRadius()},{0.0f,0.0f,0.0f},collider->GetWorldPosition());
		Renderer::GetInstance()->DrawSphere(transform, "white_template", {1.0f,1.0f,1.0f,1.0f});
	}
}

void CollisionManager::RegisterGlobalVariables() {
	const std::string name = "Collider";
	GlobalVariables::GetInstance()->CreateGroup(name);
	GlobalVariables::GetInstance()->AddValue(name, "colliderVisible", isColliderDraw_);
};

void CollisionManager::ApplyGlobalVariables() {
	const std::string name = "Collider";
	isColliderDraw_ = GlobalVariables::GetInstance()->GetBoolValue(name, "colliderVisible");
};