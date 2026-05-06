#include "Enemy.h"
#include "Player.h"

void Enemy::Initialize(const Vector3& position) {
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("enemy"));
	transform_.Initialize();
	transform_.translate = position;
	transform_.rotate.y = Radian(-90);

	velocity_ = { -kWalkSpeed,0.0f,0.0f };

	walkTimer_ = 0.0f;
}

void Enemy::Update() {
	transform_.translate += velocity_;

	WalkAnimationUpdate();
}

void Enemy::Draw() {
	model_.Draw(transform_);
}

Vector3 Enemy::GetWorldPosition() {
	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform_);

	Vector3 worldPos;

	worldPos.x = worldMatrix.matrix[3][0];
	worldPos.y = worldMatrix.matrix[3][1];
	worldPos.z = worldMatrix.matrix[3][2];


	return worldPos;
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f,worldPos.y - kHeight / 2.0f,worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f,worldPos.y + kHeight / 2.0f,worldPos.z + kWidth / 2.0f };

	return aabb;
}

void Enemy::OnCollision(const Player* player) {
	(void)player;
}

void Enemy::WalkAnimationUpdate() {
	walkTimer_ += 1.0f / 60.0f;

	float param = sin((2.0f * std::numbers::pi_v<float>) * walkTimer_ / kWalkMotionTime);
	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	transform_.rotate.x = Radian(degree);

}
