#include "FollowCamera.h"

FollowCamera* FollowCamera::GetInstance() {
	static FollowCamera instance;
	return &instance;
}

void FollowCamera::Initialize() {
	transform_.Initialize();
	isMove_ = false;

	angleDirection_ = 0.0f;
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
	float newAngleDirection = 0.0f;


	interTarget_ = Lerp(interTarget_, target_->translate, kCompletionRate);
	float direction = 0.0f;
	if (isMove_) {
		direction = std::fabs(Degree(transform_.rotate.y - target_->rotate.y));
		if (direction >= 180.0f) {
			direction = std::fabs(direction - 360.0f);
			if (destinationAngleY_ >= 0.0f) {
				transform_.rotate.y -= Radian(360.0f);
				destinationAngleY_ -= Radian(360.0f);
			} else {
				destinationAngleY_ += Radian(360.0f);
				transform_.rotate.y += Radian(360.0f);
			}
		}

		if (direction <= kLerpPlayerDirectionMin_) {
			destinationAngleY_ = Lerp(destinationAngleY_, target_->rotate.y, Easing(0.0f, kAutoCompletionRate, direction, kLerpPlayerDirectionMin_, EaseType::kConstant));
		} else if (direction <= kLerpPlayerDirectionMax_) {
			destinationAngleY_ = Lerp(destinationAngleY_, target_->rotate.y, kAutoCompletionRate);
		} else if (direction <= kLerpPlayerDirectionEase_) {
			destinationAngleY_ = Lerp(destinationAngleY_, target_->rotate.y, Easing(kAutoCompletionRate, 0.0f, direction - kLerpPlayerDirectionMax_, kLerpPlayerDirectionEase_ - kLerpPlayerDirectionMax_, EaseType::kConstant));
		}
	}

	ImGui::Begin("aa");
	ImGui::Text("%f,%f,%f,%f", direction, destinationAngleY_, target_->rotate.y, Degree(transform_.rotate.y - target_->rotate.y));
	ImGui::End();

	transform_.rotate.y = Lerp(transform_.rotate.y, destinationAngleY_, kCompletionRate);

	transform_.translate = interTarget_ + offset;

	if (std::fabs(transform_.rotate.y) >= Radian(360.0f)) {
		if (transform_.rotate.y >= 0.0f) {
			transform_.rotate.y -= Radian(360.0f);
			destinationAngleY_ -= Radian(360.0f);
		} else {
			transform_.rotate.y += Radian(360.0f);
			destinationAngleY_ += Radian(360.0f);
		}
		//transform_.rotate.y = std::fmod(transform_.rotate.y, Radian(360.0f));
	}

	preTargetRotateY_ = target_->rotate.y;

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
