#include "CameraController.h"
#include "Player.h"

void CameraController::Initialize() {
	Reset();
}

void CameraController::Update() {
	const Transform& targetTransform = target_->GetTransform();
	afterPosition_ = static_cast<Vector3>(targetTransform.translate) + targetOffset_ + (target_->GetVelocity() *kVelocityBias);

	Vector3 cameraPosition = Lerp(Camera::GetInstance()->GetPosition(), afterPosition_, kInterpolationRate);

	cameraPosition.x = std::max(cameraPosition.x,target_->GetTransform().translate.x + kMargin.left);
	cameraPosition.x = std::min(cameraPosition.x,target_->GetTransform().translate.x + kMargin.right);
	cameraPosition.y = std::max(cameraPosition.y,target_->GetTransform().translate.y + kMargin.botom);
	cameraPosition.y = std::min(cameraPosition.y,target_->GetTransform().translate.y + kMargin.top);

	cameraPosition.x = std::max(cameraPosition.x,movableArea_.left);
	cameraPosition.x = std::min(cameraPosition.x,movableArea_.right);
	cameraPosition.y = std::max(cameraPosition.y,movableArea_.botom);
	cameraPosition.y = std::min(cameraPosition.y,movableArea_.top);

	Camera::GetInstance()->SetPosition(cameraPosition);

	Camera::GetInstance()->Update();
}

void CameraController::Reset() {
	const Transform& targetTransform = target_->GetTransform();

	Camera::GetInstance()->SetPosition(static_cast<Vector3>(targetTransform.translate) + targetOffset_);
}