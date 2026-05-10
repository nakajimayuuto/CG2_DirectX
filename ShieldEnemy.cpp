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
		case ShieldEnemy::Behavior::kGuard:
			BehaviorGuardInitialize();
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
	case ShieldEnemy::Behavior::kGuard:
		BehaviorGuardUpdate();
		break;
	}
}

void ShieldEnemy::RegisterGlobalVariables() {
	const char* groupName = "ShieldEnemy";

	GlobalVariables::GetInstance()->CreateGroup(groupName);

	GlobalVariables::GetInstance()->AddValue(groupName, "DeathAnimationParameterSpin", kDeathAnimationParameterSpin);
	GlobalVariables::GetInstance()->AddValue(groupName, "DeathAnimationParameterShrink", kDeathAnimationParameterShrink);

	GlobalVariables::GetInstance()->AddValue(groupName, "GuardParameterBack", kGuardParameterBack);
	GlobalVariables::GetInstance()->AddValue(groupName, "GuardParameterStop", kGuardParameterStop);

	GlobalVariables::GetInstance()->AddValue(groupName, "WalkSpeed", kWalkSpeed);

	GlobalVariables::GetInstance()->AddValue(groupName, "Width", kWidth);
	GlobalVariables::GetInstance()->AddValue(groupName, "Height", kHeight);

	GlobalVariables::GetInstance()->AddValue(groupName, "WalkMotionAngleStart", kWalkMotionAngleStart);

	GlobalVariables::GetInstance()->AddValue(groupName, "WalkMotionAngleEnd", kWalkMotionAngleEnd);

	GlobalVariables::GetInstance()->AddValue(groupName, "WalkMotionTime", kWalkMotionTime);
}

void ShieldEnemy::ApplyGlobalVariables() {
	const char* groupName = "ShieldEnemy";
	kDeathAnimationParameterSpin = GlobalVariables::GetInstance()->GetFloatValue(groupName, "DeathAnimationParameterSpin");
	kDeathAnimationParameterShrink = GlobalVariables::GetInstance()->GetFloatValue(groupName, "DeathAnimationParameterShrink");

	kGuardParameterBack = GlobalVariables::GetInstance()->GetFloatValue(groupName, "GuardParameterBack");
	kGuardParameterStop = GlobalVariables::GetInstance()->GetFloatValue(groupName, "GuardParameterStop");

	kWalkSpeed = GlobalVariables::GetInstance()->GetFloatValue(groupName, "WalkSpeed");

	kWidth = GlobalVariables::GetInstance()->GetFloatValue(groupName, "Width");
	kHeight = GlobalVariables::GetInstance()->GetFloatValue(groupName, "Height");

	kWalkMotionAngleStart = GlobalVariables::GetInstance()->GetFloatValue(groupName, "WalkMotionAngleStart");

	kWalkMotionAngleEnd = GlobalVariables::GetInstance()->GetFloatValue(groupName, "WalkMotionAngleEnd");

	kWalkMotionTime = GlobalVariables::GetInstance()->GetFloatValue(groupName, "WalkMotionTime");
}

void ShieldEnemy::BehaviorRootInitialize() {

	velocity_ = { -kWalkSpeed,0.0f,0.0f };
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

void ShieldEnemy::BehaviorGuardInitialize() {
	guardPhase_ = GuardPhase::kBack;

	guardParameter_ = 0.0f;
}

void ShieldEnemy::BehaviorGuardUpdate() {
	guardParameter_ += 1.0f / 60.0f;

	switch (guardPhase_) {
	case ShieldEnemy::GuardPhase::kBack:
		if (lrDirection_ == LRDirection::kRight) {
			velocity_.x = Easing(-0.1f, 0.0f, guardParameter_, kGuardParameterBack, EaseType::kEaseOut);
		} else {
			velocity_.x = Easing(0.1f, 0.0f, guardParameter_, kGuardParameterBack, EaseType::kEaseOut);
		}

		transform_.rotate.x = Easing(0.0f, Radian(-15.0f), guardParameter_, kGuardParameterBack, EaseType::kEaseIn);

		if (guardParameter_ >= kGuardParameterBack) {
			guardPhase_ = GuardPhase::kStop;
			guardParameter_ = 0.0f;
		}
		break;
	case ShieldEnemy::GuardPhase::kStop:
		transform_.rotate.x = Easing(Radian(-15.0f), 0.0f, guardParameter_, kGuardParameterStop, EaseType::kEaseOut);

		if (guardParameter_ >= kGuardParameterStop) {
			behaviorRequest_ = Behavior::kRoot;
			guardParameter_ = 0.0f;
		}
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
	transform_.rotate.y = Radian(degree - 90.0f);

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

			behaviorRequest_ = Behavior::kGuard;
			return;
		}

		behaviorRequest_ = Behavior::kDeathAnimation;

		scene->CreateEffect(effectPos, BaseEffect::EffectType::kHit);
	}
}
