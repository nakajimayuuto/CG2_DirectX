#include "Dummy.h"

void Dummy::Initialize(const Vector3& position) {
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("dummy"));
	transform_.Initialize();
	transform_.translate = position;
	transform_.translate.y = transform_.translate.y + kStartPosY;
	colliderType_ = ColliderType::kSphere;
	colliderRadius_ = 1.0f;
	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer);
	collisionMask_ =
		CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack) |
		CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer);

	velocity_ = { -kWalkSpeed,0.0f,0.0f };

	walkTimer_ = 0.0f;

	isCollisionDisable_ = false;
}

void Dummy::Update() {
	if (behaviorRequest_ != Behavior::kUnknown) {
		behavior_ = behaviorRequest_;

		switch (behavior_) {
		case Dummy::Behavior::kRoot:
			BehaviorRootInitialize();
			break;
		case Dummy::Behavior::kDeathAnimation:
			break;
		}

		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Dummy::Behavior::kRoot:
		BehaviorRootUpdate();
		break;
	case Dummy::Behavior::kDeathAnimation:
		break;
	}

	DrawCollider();
}

void Dummy::BehaviorRootInitialize() {
}

void Dummy::BehaviorRootUpdate() {
	//transform_.translate += velocity_;
}

void Dummy::Draw() {
	model_.Draw(transform_);
}

void Dummy::OnCollision(Collider* other){
	Transform newTransform;
	newTransform = transform_;
	newTransform.translate.y += kDamageEffectPosY;
	ParticleManager::GetInstance()->SpawnNumbers(other->GetDamage(), newTransform, {0.5f,0.5f,1.0f});
	other->SetActive(false);
}
