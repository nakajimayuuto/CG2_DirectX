#include "NormalBullet.h"

void NormalBullet::Initialize(const std::string& modelName, const Vector3& position, const Vector3& velocity){
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

void NormalBullet::Update(){
	LifeTimeUpdate();

	transform_.translate += velocity_;
}

void NormalBullet::LifeTimeUpdate() {
	deathTimer_ -= 1.0f / 60.0f;

	if (deathTimer_ <= 0.0f) {
		isActive_ = false;
	}

}

void NormalBullet::Draw(){
	model_.Draw(transform_);
}

void NormalBullet::OnCollision(){
	isActive_ = false;
}

void NormalBullet::RegisterGlobalVariables() {
	const std::string name = "playerBullet";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	globalVariables->AddValue(name, "kLifeTime", kLifeTime);
}

void NormalBullet::ApplyGlobalVariables() {
	const std::string name = "playerBullet";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	kLifeTime = globalVariables->GetFloatValue(name, "kLifeTime");

}
