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
		destinationAngleY_ += input->GetRightStickDirection().x * kRotateSpeed;

		if (input->TriggerPadButton(PadButtons::INPUT_R3)) {
			Reset();
		}
	} else {
		input->SetIsCursorFixed(true);
		input->SetIsCursorVisible(false);
		destinationAngleY_ += input->GetMouse().GetMove().x * kMouseRotateSpeed;

		if (input->TriggerKey(DIK_C)) {
			Reset();
		}
	}

	Vector3 offset = GetOffset();

	interTarget_ = Lerp(interTarget_, target_->translate, kCompletionRate);
	transform_.rotate.y = Lerp(transform_.rotate.y, destinationAngleY_, kCompletionRate);

	transform_.translate = interTarget_ + offset;

	Camera::GetInstance()->SetTransform(transform_);
}

void FollowCamera::Reset() {
	if (!target_) {
		return;
	}
	interTarget_ = target_->translate;

	transform_.rotate.y = target_->rotate.y;

	destinationAngleY_ = transform_.rotate.y;

	Vector3 offset = GetOffset();

	transform_.translate = interTarget_ + offset;

	Camera::GetInstance()->SetTransform(transform_);
}

Vector3 FollowCamera::GetOffset() const {
	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateYMatrix(transform_.rotate.y);
	return cameraRotateMatrix.TransformNomal(kOffset);
}
