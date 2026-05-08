#include "Enemy.h"
#include "Player.h"
#include "./Scene/GameScene.h"

void Enemy::Initialize(const Vector3& position) {
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("enemy"));
	transform_.Initialize();
	transform_.translate = position;
	transform_.rotate.y = Radian(-90);

	velocity_ = { -kWalkSpeed,0.0f,0.0f };

	walkTimer_ = 0.0f;

	isCollisionDisable_ = false;
}

void Enemy::Update() {
	if (behaviorRequest_ != Behavior::kUnknown) {
		behavior_ = behaviorRequest_;

		switch (behavior_) {
		case Enemy::Behavior::kRoot:
			BehaviorRootInitialize();
			break;
		case Enemy::Behavior::kDeathAnimation:
			BehaviorDeathAnimationInitialize();
			break;
		}

		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Enemy::Behavior::kRoot:
		BehaviorRootUpdate();
		break;
	case Enemy::Behavior::kDeathAnimation:
		BehaviorDeathAnimationUpdate();
		break;
	}
}

void Enemy::BehaviorRootInitialize() {
}

void Enemy::BehaviorRootUpdate() {
	transform_.translate += velocity_;

	WalkAnimationUpdate();
}

void Enemy::BehaviorDeathAnimationInitialize() {
	deathAnimationPhase_ = DeathAnimationPhase::kSpin;

	deathAnimationParameter_ = 0.0f;

	velocity_ = { 0.0f ,-0.05f,0.0f};

	isCollisionDisable_ = true;
}

void Enemy::BehaviorDeathAnimationUpdate() {
	float spinSpeed;
	deathAnimationParameter_ += 1.0f / 60.0f;

	switch (deathAnimationPhase_){
	case Enemy::DeathAnimationPhase::kSpin:
		spinSpeed = Easing(5.0f, 0.0f, deathAnimationParameter_, kDeathAnimationParameterSpin, EaseType::kEaseOut);

		transform_.rotate.y += spinSpeed;

		if (deathAnimationParameter_ >= kDeathAnimationParameterSpin) {
			deathAnimationPhase_ = DeathAnimationPhase::kShrink;
			deathAnimationParameter_ = 0.0f;
		}
		break;
	case Enemy::DeathAnimationPhase::kShrink:
		transform_.scale = Easing({ 1.0f,1.0f,1.0f }, {0.0f,0.0f,0.0f}, deathAnimationParameter_, kDeathAnimationParameterShrink, EaseType::kEaseOut);
		transform_.translate += velocity_;

		if (deathAnimationParameter_ >= kDeathAnimationParameterShrink) {
			deathAnimationPhase_ = DeathAnimationPhase::kDeath;
			deathAnimationParameter_ = 0.0f;
		}
		break;
	case Enemy::DeathAnimationPhase::kDeath:
		isDead_ = true;
		break;
	}
}

void Enemy::Draw() {
	model_.Draw(transform_);
}

void Enemy::WalkAnimationUpdate() {
	walkTimer_ += 1.0f / 60.0f;

	float param = sin((2.0f * std::numbers::pi_v<float>) * walkTimer_ / kWalkMotionTime);
	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	transform_.rotate.x = Radian(degree);

}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f,worldPos.y - kHeight / 2.0f,worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f,worldPos.y + kHeight / 2.0f,worldPos.z + kWidth / 2.0f };

	return aabb;
}

void  Enemy::OnCollision(GameScene* scene, const Player* player) {
	if (behavior_ == Behavior::kDeathAnimation) {
		return;
	}

	if (player->IsAttack()) {
		behaviorRequest_ = Behavior::kDeathAnimation;

		Vector3 effectPos = ((GetWorldPosition() + player->GetWorldPosition())) * 0.5f;
		scene->CreateHitEffect(effectPos);
	}
}
