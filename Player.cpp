#include "Player.h"

void Player::Initialize() {
	transform_.Initialize();
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
}

void Player::Update() {

}

void Player::Draw() {
	model_.Draw(transform_);
}