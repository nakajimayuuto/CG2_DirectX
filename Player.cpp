#include "Player.h"

void Player::Initialize() {
	transform_.Initialize();

	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
}

void Player::Update() {
	Vector3 move = { 0.0f,0.0f,0.0f };
	InputManager* input = InputManager::GetInstance();
	if (input->IsGamePadConnect()) {
		move = {input->GetLeftStickDirection().x,0.0f,input->GetLeftStickDirection().y};

		move = move.Normalize() * kSpeed;
	} else {
		if (input->PressKey(DIK_W)) {
			move.z += 1.0f;
		}

		if (input->PressKey(DIK_S)) {
			move.z -= 1.0f;
		}
		
		if (input->PressKey(DIK_A)) {
			move.x -= 1.0f;
		}
		
		if (input->PressKey(DIK_D)) {
			move.x += 1.0f;
		}

		move = move.Normalize() * kSpeed;
	}

	transform_.translate += move;

}

void Player::Draw() {
	Renderer::GetInstance()->DrawModel(transform_,&model_);
}