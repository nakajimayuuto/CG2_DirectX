#include "Player.h"

void Player::Initialize() {
	transform_.Initialize();
	transform_.translate.y = 1.0f;
	targetRotateY = 0.0f;
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));

	//transform_.rotate.y = std::atan2(velocity.x, velocity.z);
	//Vector3 velocityXZ = { velocity.x,0.0f,velocity.z };
	//transform_.rotate.x = std::atan2(-velocity.y, velocityXZ.Length());
}

void Player::Update() {
	isMoving_ = false;

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

	if (move.x != 0.0f && move.z != 0.0f) {
		isMoving_ = true;
	}

	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateMatrix(Camera::GetInstance()->GetTransform().rotate);

	move = cameraRotateMatrix.TransformNomal(move);

	if (isMoving_) {
		targetRotateY = std::atan2(move.x, move.z);
	}
	
	transform_.rotate.y = LerpShortAngle(transform_.rotate.y,targetRotateY,kCompletionRate);
	//transform_.rotate.y = std::atan2(move.x, move.z);

	transform_.translate += move;

}

void Player::Draw() {
	Renderer::GetInstance()->DrawModel(transform_,&model_);
}