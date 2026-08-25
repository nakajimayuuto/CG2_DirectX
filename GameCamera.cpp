#include "GameCamera.h"

GameCamera* GameCamera::GetInstance() {
	static GameCamera instance;
	return &instance;
}

void GameCamera::Initialize() {
	transform_.Initialize();
	isMove_ = false;
	isDash_ = false;
	isAutoHalAttack_ = false;

	angleDirection_ = 0.0f;

	destinationPlayerAngleY_ = 0.0f;
	destinationDashAngleY_ = 0.0f;
	destinationTargetAngleY_ = 0.0f;
	destinationAngleY_ = 0.0f;

	dashEaseTimer_ = 0.0f;
	targetEaseTimer_ = 0.0f;
}

void GameCamera::Update() {
	if (gGamePhase == GamePhase::kGameStartAnim ||
		gGamePhase == GamePhase::kBossPhaseChangeAnim ||
		gGamePhase == GamePhase::kBossLastJaronaAnim
		) {

	} else {
		FollowedUpdate();
	}

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

void GameCamera::FollowedUpdate() {
	if (!target_) {
		return;
	}

	FollowedControlAction();

	FollowedDash();

	FollowedTarget();

	if (isDash_) {
		Camera::GetInstance()->SetFovY(Lerp(Camera::GetInstance()->GetFovY(), kDashFovY, 0.05f));
		if (dashEaseTimer_ >= 1.0f) {
			dashEaseTimer_ = 1.0f;
		} else {
			dashEaseTimer_ += DeltaTime::GetInstance()->GetGameTime();
		}

		if (targetEaseTimer_ <= 0.0f) {
			targetEaseTimer_ = 0.0f;
		} else {
			targetEaseTimer_ -= DeltaTime::GetInstance()->GetGameTime();
		}
		//} else if((static_cast<Vector3>(targetEnemy_->translate) - static_cast<Vector3>(target_->translate)).Length() < 30.0f){
	} else if (isAutoHalAttack_) {
		Camera::GetInstance()->SetFovY(Lerp(Camera::GetInstance()->GetFovY(), kNormalFovY, 0.05f));
		if (dashEaseTimer_ <= 0.0f) {
			dashEaseTimer_ = 0.0f;
		} else {
			dashEaseTimer_ -= DeltaTime::GetInstance()->GetGameTime();
		}

		if (targetEaseTimer_ <= 0.0f) {
			targetEaseTimer_ = 0.0f;
		} else {
			targetEaseTimer_ -= DeltaTime::GetInstance()->GetGameTime();
		}

	} else {
		//destinationAngleY_ = destinationTargetAngleY_;// LerpShortAngle(destinationAngleY_, destinationPlayerAngleY_, kCompletionRate);


		if (dashEaseTimer_ <= 0.0f) {
			dashEaseTimer_ = 0.0f;
		} else {
			dashEaseTimer_ -= DeltaTime::GetInstance()->GetGameTime();
		}

		if (targetEaseTimer_ >= 1.0f) {
			targetEaseTimer_ = 1.0f;
		} else {
			targetEaseTimer_ += DeltaTime::GetInstance()->GetGameTime();
		}
		Camera::GetInstance()->SetFovY(Lerp(Camera::GetInstance()->GetFovY(), kNormalFovY, 0.05f));
		//destinationAngleY_ = std::atan2(targetEnemy_->translate.x - transform_.translate.x, targetEnemy_->translate.z - transform_.translate.z);
	}

	if (gGamePhase != GamePhase::kTutorial) {
		destinationPlayerAngleY_ = LerpShortAngle(destinationPlayerAngleY_, destinationTargetAngleY_, targetEaseTimer_);
	}

	destinationAngleY_ = LerpShortAngle(destinationPlayerAngleY_, destinationDashAngleY_, dashEaseTimer_);

	destinationPlayerAngleY_ = destinationAngleY_;
	destinationTargetAngleY_ = destinationAngleY_;
	destinationDashAngleY_ = destinationAngleY_;

	transform_.translate = interOffsetTarget_;

	//if (gGamePhase == GamePhase::kTutorial || gGamePhase == GamePhase::kBossLastJarona || gGamePhase == GamePhase::kGameClearStage) {
	if (gGamePhase == GamePhase::kTutorial) {
		TutorialWallClamp();
	} else {
		FollowedWallClamp();
	}

	transform_.rotate.y = LerpShortAngle(transform_.rotate.y, destinationAngleY_, kCompletionRate);

	//if (std::fabs(transform_.rotate.y) >= Radian(360.0f)) {
	//	if (transform_.rotate.y >= 0.0f) {
	//		transform_.rotate.y -= Radian(360.0f);
	//		destinationAngleY_ -= Radian(360.0f);
	//	} else {
	//		transform_.rotate.y += Radian(360.0f);
	//		destinationAngleY_ += Radian(360.0f);
	//	}
	//}

	// ここまで地獄

	preTargetRotateY_ = target_->rotate.y;
}

void GameCamera::FollowedControlAction() {
	InputManager* input = InputManager::GetInstance();

	if (input->TriggerKey(DIK_E)) {
		isMove_ = isMove_;
	}

	if (input->IsGamePadConnect()) {
		input->SetIsCursorFixed(false);
		input->SetIsCursorVisible(true);
		destinationPlayerAngleY_ += input->GetRightStickDirection().x * kRotateSpeed;

		if (input->TriggerPadButton(PadButtons::INPUT_R3)) {
			Reset();
		}
	} else {
		//input->SetIsCursorFixed(true);
		//input->SetIsCursorVisible(false);
		destinationPlayerAngleY_ += input->GetMouse().GetMove().x * kMouseRotateSpeed;

		if (input->TriggerKey(DIK_C)) {
			Reset();
		}
	}

	Vector3 offset = GetOffset();
	float newAngleDirection = 0.0f;

	float targetY = interTarget_.y;
	interTarget_ = Lerp(interTarget_, target_->translate, kCompletionRate);
	float direction = 0.0f;
	interTarget_.y = targetY;


	distanceToCenter_ = (interTarget_ + offset).Length();

	if (distanceToCenter_ <= movingRadius_) {
		if (isMove_) {
			distanceToCenter_ = std::fabs(Degree(transform_.rotate.y - target_->rotate.y));
			if (distanceToCenter_ <= kLerpPlayerDirectionMin_) {
				destinationPlayerAngleY_ = Lerp(destinationPlayerAngleY_, target_->rotate.y, Easing(0.0f, kAutoCompletionRate, distanceToCenter_, kLerpPlayerDirectionMin_, EaseType::kConstant));
			} else if (distanceToCenter_ <= kLerpPlayerDirectionMax_) {
				destinationPlayerAngleY_ = Lerp(destinationPlayerAngleY_, target_->rotate.y, kAutoCompletionRate);
			} else if (distanceToCenter_ <= kLerpPlayerDirectionEase_) {
				destinationPlayerAngleY_ = Lerp(destinationPlayerAngleY_, target_->rotate.y, Easing(kAutoCompletionRate, 0.0f, distanceToCenter_ - kLerpPlayerDirectionMax_, kLerpPlayerDirectionEase_ - kLerpPlayerDirectionMax_, EaseType::kConstant));
			} else {
				if (destinationPlayerAngleY_ >= 0.0f) {
					transform_.rotate.y -= Radian(360.0f);
					destinationPlayerAngleY_ -= Radian(360.0f);
				} else {
					destinationPlayerAngleY_ += Radian(360.0f);
					transform_.rotate.y += Radian(360.0f);
				}

				if (distanceToCenter_ <= kLerpPlayerDirectionMin_) {
					destinationPlayerAngleY_ = Lerp(destinationPlayerAngleY_, target_->rotate.y, Easing(0.0f, kAutoCompletionRate, distanceToCenter_, kLerpPlayerDirectionMin_, EaseType::kConstant));
				} else if (distanceToCenter_ <= kLerpPlayerDirectionMax_) {
					destinationPlayerAngleY_ = Lerp(destinationPlayerAngleY_, target_->rotate.y, kAutoCompletionRate);
				} else if (distanceToCenter_ <= kLerpPlayerDirectionEase_) {
					destinationPlayerAngleY_ = Lerp(destinationPlayerAngleY_, target_->rotate.y, Easing(kAutoCompletionRate, 0.0f, distanceToCenter_ - kLerpPlayerDirectionMax_, kLerpPlayerDirectionEase_ - kLerpPlayerDirectionMax_, EaseType::kConstant));
				}
			}
		}
	}

	interOffsetTarget_ = interTarget_ + offset;
}

void GameCamera::TutorialWallClamp() {
	transform_.translate.x = 0.0f;
	transform_.rotate.y = 0.0f;
	destinationAngleY_ = 0.0f;

}

void GameCamera::FollowedWallClamp() {
	/// ここから地獄
	distanceToCenter_ = interOffsetTarget_.Length();

	Vector3 wallOffsetPos = interOffsetTarget_;

	float wallDirection = 0.0f;
	if (distanceToCenter_ > movingRadius_) {
		wallOffsetPos = interOffsetTarget_.Normalize() * movingRadius_;
		transform_.translate = Lerp(interOffsetTarget_, wallOffsetPos, 0.75f);
		//wallDirection = std::atan2(target_->translate.x - transform_.translate.x, target_->translate.z - transform_.translate.z);
		//
		//if (std::fabs(destinationAngleY_ - wallDirection) >= Radian(360.0f)) {
		//	if (wallDirection < 0.0f) {
		//		wallDirection += Radian(360.0f);
		//	} else {
		//		wallDirection -= Radian(360.0f);
		//	}
		//}
		//
		//destinationAngleY_ = LerpShortAngle(destinationAngleY_, wallDirection, 0.1f);
	}
}

void GameCamera::FollowedDash() {
	destinationDashAngleY_ = LerpShortAngle(destinationDashAngleY_, target_->rotate.y, 0.1f);
}

void GameCamera::FollowedTarget() {
	destinationTargetAngleY_ = std::atan2(targetEnemy_->translate.x - transform_.translate.x, targetEnemy_->translate.z - transform_.translate.z);
}

Vector3 GameCamera::GetOffset() const {
	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateYMatrix(transform_.rotate.y);
	return cameraRotateMatrix.TransformNomal(kOffset);
}
