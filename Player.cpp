#include "Player.h"

void Player::Initialize() {
	transform_.Initialize();
	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo("uvChecker"));
}

void Player::Update() {

}

void Player::Draw() {
	model_.Draw(transform_);
}