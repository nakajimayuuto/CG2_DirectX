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

	//minPosition = { 12.0f, 7.0f,-100.0f };
	minPosition = tutorialMinPos;
	maxPosition = tutorialMaxPos;

	currentAriaNumber_ = 0;

	cameraArias_[0].currentPos = tutorialMinPos;
	cameraArias_[0].minPos = tutorialMinPos;
	cameraArias_[0].maxPos = tutorialMaxPos;

	cameraArias_[1].currentPos = phase1MinPos;
	cameraArias_[1].minPos = phase1MinPos;
	cameraArias_[1].maxPos = phase1MaxPos;
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

void GameCamera::StartAriaChange(uint32_t number, float timerMax){
	isAriaChange_ = true;
	preInterOffsetTarget_ = interOffsetTarget_;
	ariaChangeTimeMax_ = timerMax;
	ariaChangeTimer_ = 0.0f;
	currentAriaNumber_ = number;

	if (
		preInterOffsetTarget_.x == cameraArias_[currentAriaNumber_].currentPos.x &&
		preInterOffsetTarget_.y == cameraArias_[currentAriaNumber_].currentPos.y &&
		preInterOffsetTarget_.z == cameraArias_[currentAriaNumber_].currentPos.z
		) {
		isAriaChange_ = false;
	}
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


	destinationPlayerAngleY_ = LerpShortAngle(destinationPlayerAngleY_, destinationTargetAngleY_, targetEaseTimer_);
	destinationAngleY_ = LerpShortAngle(destinationPlayerAngleY_, destinationDashAngleY_, dashEaseTimer_);

	destinationPlayerAngleY_ = destinationAngleY_;
	destinationTargetAngleY_ = destinationAngleY_;
	destinationDashAngleY_ = destinationAngleY_;

	//transform_.translate = interOffsetTarget_;

	//TutorialWallClamp();
	FollowedWallClamp();

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

	Vector3 offset = GetOffset();
	float newAngleDirection = 0.0f;

	float targetY = interTarget_.y;
	interTarget_ = Lerp(interTarget_, target_->translate, kCompletionRate);
	float direction = 0.0f;
	//interTarget_.y = targetY;


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
	for (size_t i = 0; i != cameraArias_.size(); ++i) {
		cameraArias_[i].currentPos = interOffsetTarget_;
		if (cameraArias_[i].currentPos.x < cameraArias_[i].minPos.x) {
			cameraArias_[i].currentPos.x = cameraArias_[i].minPos.x;
		}
		if (cameraArias_[i].currentPos.y < cameraArias_[i].minPos.y) {
			cameraArias_[i].currentPos.y = cameraArias_[i].minPos.y;
		}
		if (cameraArias_[i].currentPos.x > cameraArias_[i].maxPos.x) {
			cameraArias_[i].currentPos.x = cameraArias_[i].maxPos.x;
		}
		if (cameraArias_[i].currentPos.y > cameraArias_[i].maxPos.y) {
			cameraArias_[i].currentPos.y = cameraArias_[i].maxPos.y;
		}
	}
	interOffsetTarget_ = GetAriaChangEasePos();// Lerp(interOffsetTarget_, cameraArias_[currentAriaNumber_].currentPos, kCompletionRate);

	transform_.translate = interOffsetTarget_;

	Camera::GetInstance()->SetPosition(transform_.GetWorldPosition());

	return;
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
	if (targetEnemy_) {
		destinationTargetAngleY_ = std::atan2(targetEnemy_->translate.x - transform_.translate.x, targetEnemy_->translate.z - transform_.translate.z);
	} else {
		destinationTargetAngleY_ = destinationPlayerAngleY_;
	}
}

Vector3 GameCamera::GetOffset() const {
	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateYMatrix(transform_.rotate.y);
	return cameraRotateMatrix.TransformNomal(kOffset);
}

Vector3 GameCamera::GetAriaChangEasePos(){
	if (!isAriaChange_) {
		return cameraArias_[currentAriaNumber_].currentPos;
	}
	ariaChangeTimer_ += DeltaTime::GetInstance()->GetDeltaTime();
	if (ariaChangeTimer_ >= ariaChangeTimeMax_) {
		isAriaChange_ = false;
	}
	return Easing(preInterOffsetTarget_,cameraArias_[currentAriaNumber_].currentPos,ariaChangeTimer_,ariaChangeTimeMax_,EaseType::kEaseOut);
}
