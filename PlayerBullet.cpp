#include "PlayerBullet.h"

void PlayerBullet::Initialize(const std::string& modelName, const Vector3& position, const Vector3& velocity){
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo(modelName));
	transform_.Initialize();
	transform_.translate = position;

	transform_.scale = {0.5f,0.5f,0.5f};

	velocity_ = velocity;
	deathTimer_ = kLifeTime;
	isActive_ = true;

	SetCollisionAttribute(kCollisionAttributePlayer);
	SetCollisionMask(kCollisionAttributeEnemy);

}

void PlayerBullet::Update(){
	LifeTimeUpdate();

	transform_.translate += velocity_;
}

void PlayerBullet::LifeTimeUpdate() {
	deathTimer_ -= 1.0f / 60.0f;

	if (deathTimer_ <= 0.0f) {
		isActive_ = false;
	}

}

void PlayerBullet::Draw(){
	model_.Draw(transform_);
}

void PlayerBullet::OnCollision(){
	isActive_ = false;
}

void PlayerBullet::RegisterGlobalVariables() {
	const std::string name = "playerBullet";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	globalVariables->AddValue(name, "kLifeTime", kLifeTime);
}

void PlayerBullet::ApplyGlobalVariables() {
	const std::string name = "playerBullet";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	kLifeTime = globalVariables->GetFloatValue(name, "kLifeTime");

}
