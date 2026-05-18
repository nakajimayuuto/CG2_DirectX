#include "HomingBullet.h"
#include "Player.h"

HomingBullet::~HomingBullet() {
	delete target_;
}

void HomingBullet::Initialize(const std::string& modelName, const Vector3& position, const Vector3& velocity) {
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo(modelName));
	model_.SetColor({ 1.0f,0.0f,0.0f,1.0f });
	transform_.Initialize();
	transform_.translate = position;
	transform_.scale.x = 1.0f;
	transform_.scale.y = 1.0f;
	transform_.scale.z = 3.0f;

	transform_.rotate.y = std::atan2(velocity.x, velocity.z);
	Vector3 velocityXZ = { velocity.x,0.0f,velocity.z };
	transform_.rotate.x = std::atan2(-velocity.y, velocityXZ.Length());

	velocity_ = velocity;
	deathTimer_ = kLifeTime;
	isActive_ = true;

}

void HomingBullet::Update() {
	LifeTimeUpdate();
	HomingUpdate();
}

void HomingBullet::LifeTimeUpdate() {
	deathTimer_ -= 1.0f / 60.0f;

	if (deathTimer_ <= 0.0f) {
		isActive_ = false;
	}

}

void HomingBullet::HomingUpdate() {
	if (target_ == nullptr) {
		transform_.translate += velocity_;
		return;
	}

	Vector3 toPlayer = target_->GetWorldPosition() - GetWorldPosition();

	toPlayer = toPlayer.Normalize();

	velocity_ = velocity_.Normalize();

	velocity_ = Slerp(velocity_, toPlayer, kHomingRatio) * kSpeed;

	transform_.translate += velocity_;

	transform_.rotate.y = std::atan2(velocity_.x, velocity_.z);
	Vector3 velocityXZ = { velocity_.x,0.0f,velocity_.z };
	transform_.rotate.x = std::atan2(-velocity_.y, velocityXZ.Length());
}

void HomingBullet::Draw() {
	model_.Draw(transform_);
}

void HomingBullet::OnCollision() {
	isActive_ = false;
}

void HomingBullet::RegisterGlobalVariables() {
	const std::string name = "EnemyBullet";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	globalVariables->AddValue(name, "kLifeTime", kLifeTime);
}

void HomingBullet::ApplyGlobalVariables() {
	const std::string name = "EnemyBullet";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	kLifeTime = globalVariables->GetFloatValue(name, "kLifeTime");

}