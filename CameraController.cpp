#include "CameraController.h"
#include "Player.h"

void CameraController::Initialize() {
	Reset();
}

void CameraController::Update() {
	switch (mode_) {
	case CameraController::Mode::kFollow:
		FollowUpdate();
		break;
	case CameraController::Mode::kForcedScroll:
		ForcedScrollUpdate();
		break;
	}


	Camera::GetInstance()->Update();
}

void CameraController::FollowUpdate() {
	const Transform& targetTransform = target_->GetTransform();
	afterPosition_ = static_cast<Vector3>(targetTransform.translate) + targetOffset_ + (target_->GetVelocity() * kVelocityBias);

	cameraPosition_ = Lerp(Camera::GetInstance()->GetPosition(), afterPosition_, kInterpolationRate);

	cameraPosition_.x = std::max(cameraPosition_.x, target_->GetTransform().translate.x + kMargin.left);
	cameraPosition_.x = std::min(cameraPosition_.x, target_->GetTransform().translate.x + kMargin.right);
	cameraPosition_.y = std::max(cameraPosition_.y, target_->GetTransform().translate.y + kMargin.botom);
	cameraPosition_.y = std::min(cameraPosition_.y, target_->GetTransform().translate.y + kMargin.top);

	cameraPosition_.x = std::max(cameraPosition_.x, movableArea_.left);
	cameraPosition_.x = std::min(cameraPosition_.x, movableArea_.right);
	cameraPosition_.y = std::max(cameraPosition_.y, movableArea_.botom);
	cameraPosition_.y = std::min(cameraPosition_.y, movableArea_.top);

	Camera::GetInstance()->SetPosition(cameraPosition_);
}

void CameraController::ForcedScrollUpdate() {
	cameraPosition_ += velocity_;

	cameraPosition_.x = std::max(cameraPosition_.x, movableArea_.left);
	cameraPosition_.x = std::min(cameraPosition_.x, movableArea_.right);
	cameraPosition_.y = std::max(cameraPosition_.y, movableArea_.botom);
	cameraPosition_.y = std::min(cameraPosition_.y, movableArea_.top);

	Vector3 pos = target_->GetTransform().translate;

	if (pos.x <= cameraPosition_.x - kCameraEndBlank_) {
		Player::CollisionMapInfo scrollmapChipInfo;

		scrollmapChipInfo.movementAmount = { velocity_.x,0.0f,0.0f };

		target_->ScrollCollision(scrollmapChipInfo);

		if (scrollmapChipInfo.isWallCollision) {
			velocity_.x = 0.0f;
			target_->PlayKDeathMotion();
		}

		if (target_->GetVelocity().x < 0.0f) {
			target_->SetPosition({ cameraPosition_.x - kCameraEndBlank_ + velocity_.x,pos.y + velocity_.y,pos.z + velocity_.z});
		}
	}

	if (pos.x >= cameraPosition_.x + kCameraEndBlank_) {
		target_->SetPosition({cameraPosition_.x + kCameraEndBlank_,pos.y,pos.z});
	}

	Camera::GetInstance()->SetPosition(cameraPosition_);
}

void CameraController::Reset() {
	const Transform& targetTransform = target_->GetTransform();

	cameraPosition_ = static_cast<Vector3>(targetTransform.translate) + targetOffset_;

	Camera::GetInstance()->SetPosition(static_cast<Vector3>(targetTransform.translate) + targetOffset_);

}

void CameraController::RegisterGlobalVariables() {

	const char* groupName = "CameraController";

	GlobalVariables::GetInstance()->CreateGroup(groupName);

	GlobalVariables::GetInstance()->AddItem(groupName, "InterpolationRate", kInterpolationRate);

	GlobalVariables::GetInstance()->AddItem(groupName, "VelocityBias", kVelocityBias);

	GlobalVariables::GetInstance()->AddItem(groupName, "Margin.botom", kMargin.botom);
	GlobalVariables::GetInstance()->AddItem(groupName, "Margin.top", kMargin.top);
	GlobalVariables::GetInstance()->AddItem(groupName, "Margin.left", kMargin.left);
	GlobalVariables::GetInstance()->AddItem(groupName, "Margin.right", kMargin.right);
}

void CameraController::ApplyGlobalVariables() {
	const char* groupName = "CameraController";

	kInterpolationRate = GlobalVariables::GetInstance()->GetFloatValue(groupName, "InterpolationRate");

	kVelocityBias = GlobalVariables::GetInstance()->GetFloatValue(groupName, "VelocityBias");

	kMargin.botom = GlobalVariables::GetInstance()->GetFloatValue(groupName, "Margin.botom");
	kMargin.top = GlobalVariables::GetInstance()->GetFloatValue(groupName, "Margin.top");
	kMargin.left = GlobalVariables::GetInstance()->GetFloatValue(groupName, "Margin.left");
	kMargin.right = GlobalVariables::GetInstance()->GetFloatValue(groupName, "Margin.right");
}