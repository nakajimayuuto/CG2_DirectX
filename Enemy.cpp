#include "Enemy.h"

void Enemy::Initialize(){
	models_["enemy"].Initialize("enemy");
	transform_.Initialize();
	transform_.translate.y = 0.5f;
}

void Enemy::InitializeRotateGimmick(){
	transform_.rotate.x = 0.0f;
}

void Enemy::Update(){
	transform_.rotate.y += kRotateYSpeed;

	Vector3 move = { 0.0f,0.0f,kSpeed};

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateYMatrix(transform_.rotate.y);

	move = rotateMatrix.TransformNomal(move);

	transform_.translate += move;


	UpdateRotateGimmick();
}

void Enemy::UpdateRotateGimmick(){
	transform_.rotate.x += kAnimationRotateXSpeed;
}
