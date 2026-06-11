#include "Player.h"

void Player::Initialize() {
	transform_.Initialize();
	transform_.translate.y = 1.0f;
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

	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateMatrix(Camera::GetInstance()->GetTransform().rotate);

	move = cameraRotateMatrix.TransformNomal(move);

	transform_.translate += move;

}

void Player::Draw() {
	Renderer::GetInstance()->DrawModel(transform_,&model_);
}