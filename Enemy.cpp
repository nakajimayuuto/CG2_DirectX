#include "Enemy.h"

void Enemy::Initialize(const Vector3& position){
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("enemy"));
	transform_.Initialize();
	transform_.translate = position;
	transform_.rotate.y = Radian(-90);

	velocity_ = { -kWalkSpeed,0.0f,0.0f };

	walkTimer_ = 0.0f;
}

void Enemy::Update(){
	transform_.translate += velocity_;

	WalkAnimationUpdate();
}

void Enemy::Draw(){
	model_.Draw(transform_);
}

void Enemy::WalkAnimationUpdate(){
	walkTimer_ += 1.0f / 60.0f;

	float param = sin((2.0f * std::numbers::pi_v<float>) * walkTimer_ / kWalkMotionTime);
	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	transform_.rotate.x = Radian(degree);

}
