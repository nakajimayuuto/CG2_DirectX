#include "Player.h"

void Player::Initialize() {
	transform_.Initialize();
}

void Player::Update() {

}

void Player::Draw() {
	Renderer::GetInstance()->DrawBox(transform_, "uvChecker", {1.0f,1.0f,1.0f,1.0f});
}