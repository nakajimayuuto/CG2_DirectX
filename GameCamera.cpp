#include "GameCamera.h"

GameCamera* GameCamera::GetInstance() {
	static GameCamera instance;
	return &instance;
}

void GameCamera::Initialize() {
	transform_.Initialize();
	isMove_ = false;

	angleDirection_ = 0.0f;
}

void GameCamera::Update() {

	FollowedUpdate();

	Camera::GetInstance()->SetTransform(transform_);
}

void GameCamera::Reset() {
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

void GameCamera::FollowedUpdate(){
	if (!target_) {
		return;
	}

	InputManager* input = InputManager::GetInstance();

	if (input->TriggerKey(DIK_E)) {
		isMove_ = isMove_;
	}

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

	float distance = (interTarget_ + offset).Length();

	if (distance <= movingRadius_) {
		if (isMove_) {
			direction = std::fabs(Degree(transform_.rotate.y - target_->rotate.y));
			if (direction <= kLerpPlayerDirectionMin_) {
				destinationAngleY_ = Lerp(destinationAngleY_, target_->rotate.y, Easing(0.0f, kAutoCompletionRate, direction, kLerpPlayerDirectionMin_, EaseType::kConstant));
			} else if (direction <= kLerpPlayerDirectionMax_) {
				destinationAngleY_ = Lerp(destinationAngleY_, target_->rotate.y, kAutoCompletionRate);
			} else if (direction <= kLerpPlayerDirectionEase_) {
				destinationAngleY_ = Lerp(destinationAngleY_, target_->rotate.y, Easing(kAutoCompletionRate, 0.0f, direction - kLerpPlayerDirectionMax_, kLerpPlayerDirectionEase_ - kLerpPlayerDirectionMax_, EaseType::kConstant));
			} else {
				if (destinationAngleY_ >= 0.0f) {
					transform_.rotate.y -= Radian(360.0f);
					destinationAngleY_ -= Radian(360.0f);
				} else {
					destinationAngleY_ += Radian(360.0f);
					transform_.rotate.y += Radian(360.0f);
				}

				if (direction <= kLerpPlayerDirectionMin_) {
					destinationAngleY_ = Lerp(destinationAngleY_, target_->rotate.y, Easing(0.0f, kAutoCompletionRate, direction, kLerpPlayerDirectionMin_, EaseType::kConstant));
				} else if (direction <= kLerpPlayerDirectionMax_) {
					destinationAngleY_ = Lerp(destinationAngleY_, target_->rotate.y, kAutoCompletionRate);
				} else if (direction <= kLerpPlayerDirectionEase_) {
					destinationAngleY_ = Lerp(destinationAngleY_, target_->rotate.y, Easing(kAutoCompletionRate, 0.0f, direction - kLerpPlayerDirectionMax_, kLerpPlayerDirectionEase_ - kLerpPlayerDirectionMax_, EaseType::kConstant));
				}
			}
		}
	}

	interOffsetTarget_ = interTarget_ + offset;

	/// ここから地獄
	distance = interOffsetTarget_.Length();

	Vector3 wallOffsetPos = interOffsetTarget_;

	float wallDirection = 0.0f;
	if (distance > movingRadius_) {
		wallOffsetPos = transform_.translate.Normalize() * movingRadius_;
		transform_.translate = Lerp(interOffsetTarget_, wallOffsetPos, 0.75f);
		wallDirection = std::atan2(target_->translate.x - transform_.translate.x, target_->translate.z - transform_.translate.z);

		if (std::fabs(destinationAngleY_ - wallDirection) >= Radian(360.0f)) {
			if (wallDirection < 0.0f) {
				wallDirection += Radian(360.0f);
			} else {
				wallDirection -= Radian(360.0f);
			}
		}

		GameSystem::Log(std::format("before:{},{}\n", transform_.rotate.y, destinationAngleY_, transform_.translate.z));
		// ここ二つの値をうまくやると何とかなりそう。バグるときは大体-6.??から0.??に変換するとき
		destinationAngleY_ = LerpShortAngle(destinationAngleY_, wallDirection, 0.1f);
		//transform_.rotate.y = Lerp(transform_.rotate.y, wallDirection, 1.0f);
		GameSystem::Log(std::format("after :{},{}\n", transform_.rotate.y, destinationAngleY_, transform_.translate.z));
	} else {
		transform_.translate = Lerp(interOffsetTarget_, wallOffsetPos, 0.25f);
		GameSystem::Log(std::format("none  :{},{}\n", transform_.rotate.y, destinationAngleY_, transform_.translate.z));
	}


	transform_.rotate.y = LerpShortAngle(transform_.rotate.y, destinationAngleY_, kCompletionRate);

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
	ImGui::Begin("aa");
	ImGui::Text("%f,%f,%f", transform_.rotate.y, destinationAngleY_, wallDirection);
	ImGui::End();

	//GameSystem::Log(std::format("rotate:{},{},{}\n", transform_.rotate.x, transform_.rotate.y, transform_.rotate.z));

	// ここまで地獄

	preTargetRotateY_ = target_->rotate.y;
}

Vector3 GameCamera::GetOffset() const {
	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateYMatrix(transform_.rotate.y);
	return cameraRotateMatrix.TransformNomal(kOffset);
}
