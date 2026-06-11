#include "Player.h"

void Player::Initialize() {
	transform_.Initialize();

	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
}

void Player::Update() {

}

void Player::Draw() {
	Renderer::GetInstance()->DrawModel(transform_,&model_);
}