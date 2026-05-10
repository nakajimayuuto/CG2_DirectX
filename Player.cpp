#include "Player.h"

void Player::Initialize() {
	transform_.Initialize();
	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo("player_texture"));
}

void Player::Update() {
	InputManager* input = InputManager::GetInstance();

	Vector3 move = {0.0f,0.0f,0.0f};

	ImGui::Begin("Player");
	ImGui::DragFloat3("transform",reinterpret_cast<float*>(&transform_.translate.x),0.1f,-3.0f,3.0f);
	ImGui::End();

	if (input->PressKey(DIK_LEFT)) {
		move.x -= kCharacterSpeed;
	} else if (input->PressKey(DIK_RIGHT)) {
		move.x += kCharacterSpeed;
	}

	if (input->PressKey(DIK_UP)) {
		move.y += kCharacterSpeed;
	} else if (input->PressKey(DIK_DOWN)) {
		move.y -= kCharacterSpeed;
	}

	transform_.translate += move;

	transform_.translate.x = std::max(transform_.translate.x,-kMoveLimitX);
	transform_.translate.x = std::min(transform_.translate.x,kMoveLimitX);
	transform_.translate.y = std::max(transform_.translate.y,-kMoveLimitY);
	transform_.translate.y = std::min(transform_.translate.y,kMoveLimitY);
}

void Player::Draw() {
	model_.Draw(transform_);
}