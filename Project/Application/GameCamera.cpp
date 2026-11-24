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
	transform_.rotate.x = Radian(60.0f);
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

void GameCamera::FollowedUpdate() {
	if (!target_) {
		return;
	}


	FollowedControlAction();

}

void GameCamera::FollowedControlAction(){
	transform_.translate = static_cast<Vector3>(target_->translate) + GetOffset();

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
