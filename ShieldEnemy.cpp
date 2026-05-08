#include "ShieldEnemy.h"
#include "Player.h"
#include "./Scene/GameScene.h"

void ShieldEnemy::Initialize(const Vector3& position) {
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("shield_enemy"));
	transform_.Initialize();
	transform_.translate = position;
	transform_.rotate.y = Radian(-90);

	velocity_ = { -kWalkSpeed,0.0f,0.0f };

	walkTimer_ = 0.0f;

	isCollisionDisable_ = false;
}

void ShieldEnemy::Update() {
	if (velocity_.x > 0.0f) {
		lrDirection_ = LRDirection::kRight;
	} else {
		lrDirection_ = LRDirection::kLeft;
	}

	if (behaviorRequest_ != Behavior::kUnknown) {
		behavior_ = behaviorRequest_;

		switch (behavior_) {
		case ShieldEnemy::Behavior::kRoot:
			BehaviorRootInitialize();
			break;
		case ShieldEnemy::Behavior::kDeathAnimation:
			BehaviorDeathAnimationInitialize();
			break;
		}

		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case ShieldEnemy::Behavior::kRoot:
		BehaviorRootUpdate();
		break;
	case ShieldEnemy::Behavior::kDeathAnimation:
		BehaviorDeathAnimationUpdate();
		break;
	}
}

void ShieldEnemy::BehaviorRootInitialize() {
}

void ShieldEnemy::BehaviorRootUpdate() {
	transform_.translate += velocity_;

	WalkAnimationUpdate();
}

void ShieldEnemy::BehaviorDeathAnimationInitialize() {
	deathAnimationPhase_ = DeathAnimationPhase::kSpin;

	deathAnimationParameter_ = 0.0f;

	velocity_ = { 0.0f ,-0.05f,0.0f };

	isCollisionDisable_ = true;
}

void ShieldEnemy::BehaviorDeathAnimationUpdate() {
	float spinSpeed;
	deathAnimationParameter_ += 1.0f / 60.0f;

	switch (deathAnimationPhase_) {
	case ShieldEnemy::DeathAnimationPhase::kSpin:
		spinSpeed = Easing(5.0f, 0.0f, deathAnimationParameter_, kDeathAnimationParameterSpin, EaseType::kEaseOut);

		transform_.rotate.y += spinSpeed;

		if (deathAnimationParameter_ >= kDeathAnimationParameterSpin) {
			deathAnimationPhase_ = DeathAnimationPhase::kShrink;
			deathAnimationParameter_ = 0.0f;
		}
		break;
	case ShieldEnemy::DeathAnimationPhase::kShrink:
		transform_.scale = Easing({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, deathAnimationParameter_, kDeathAnimationParameterShrink, EaseType::kEaseOut);
		transform_.translate += velocity_;

		if (deathAnimationParameter_ >= kDeathAnimationParameterShrink) {
			deathAnimationPhase_ = DeathAnimationPhase::kDeath;
			deathAnimationParameter_ = 0.0f;
		}
		break;
	case ShieldEnemy::DeathAnimationPhase::kDeath:
		isDead_ = true;
		break;
	}
}

void ShieldEnemy::Draw() {
	model_.Draw(transform_);
}

void ShieldEnemy::WalkAnimationUpdate() {
	walkTimer_ += 1.0f / 60.0f;

	float param = sin((2.0f * std::numbers::pi_v<float>) * walkTimer_ / kWalkMotionTime);
	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	transform_.rotate.x = Radian(degree);

}

AABB ShieldEnemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f,worldPos.y - kHeight / 2.0f,worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f,worldPos.y + kHeight / 2.0f,worldPos.z + kWidth / 2.0f };

	return aabb;
}

void ShieldEnemy::OnCollision(GameScene* scene, Player* player) {
	if (behavior_ == Behavior::kDeathAnimation) {
		return;
	}

	if (player->IsAttack()) {
		Vector3 effectPos = ((GetWorldPosition() + player->GetWorldPosition())) * 0.5f;

		if ((player->GetLRDirection() == Player::LRDirection::kRight && lrDirection_ == LRDirection::kLeft) ||
			(player->GetLRDirection() == Player::LRDirection::kLeft && lrDirection_ == LRDirection::kRight)) {
			scene->CreateEffect(effectPos, BaseEffect::EffectType::kGuard);
			player->KnockBackRequest();
			return;
		}

		behaviorRequest_ = Behavior::kDeathAnimation;

		scene->CreateEffect(effectPos, BaseEffect::EffectType::kHit);
	}
}
