#include "EnemyBullet.h"
void EnemyBullet::Initialize(const std::string& modelName, const Vector3& position, const Vector3& velocity) {
	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo(modelName));
	model_.SetColor({1.0f,0.0f,0.0f,1.0f});
	transform_.Initialize();
	transform_.translate = position;
	transform_.scale.x = 0.5f;
	transform_.scale.y = 0.5f;
	transform_.scale.z = 3.0f;

	transform_.rotate.y = std::atan2(velocity.x , velocity.z);
	Vector3 velocityXZ = {velocity.x,0.0f,velocity.z};
	transform_.rotate.x = std::atan2(-velocity.y ,velocityXZ.Length());

	velocity_ = velocity;
	deathTimer_ = kLifeTime;
	isActive_ = true;

}

void EnemyBullet::Update() {
	LifeTimeUpdate();

	transform_.translate += velocity_;
}

void EnemyBullet::LifeTimeUpdate() {
	deathTimer_ -= 1.0f / 60.0f;

	if (deathTimer_ <= 0.0f) {
		isActive_ = false;
	}

}

void EnemyBullet::Draw() {
	model_.Draw(transform_);
}

void EnemyBullet::RegisterGlobalVariables() {
	const std::string name = "EnemyBullet";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	globalVariables->AddValue(name, "kLifeTime", kLifeTime);
}

void EnemyBullet::ApplyGlobalVariables() {
	const std::string name = "EnemyBullet";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	kLifeTime = globalVariables->GetFloatValue(name, "kLifeTime");

}