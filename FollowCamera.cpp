#include "FollowCamera.h"

void FollowCamera::Initialize() {
	transform_.Initialize();
}

void FollowCamera::Update() {
	if (!target_) {
		return;
	}

	InputManager* input = InputManager::GetInstance();

	if (input->IsGamePadConnect()) {
		input->SetIsCursorFixed(false);
		input->SetIsCursorVisible(true);
		transform_.rotate.y += input->GetRightStickDirection().x * kRotateSpeed;
	} else {
		input->SetIsCursorFixed(true);
		input->SetIsCursorVisible(false);
		transform_.rotate.y += input->GetMouse().GetMove().x * kMouseRotateSpeed;
	}
	
	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateMatrix(transform_.rotate);

	Vector3 offset = cameraRotateMatrix.TransformNomal(kOffset);

	transform_.translate = static_cast<Vector3>(target_->translate) + offset;

	Camera::GetInstance()->SetTransform(transform_);
}