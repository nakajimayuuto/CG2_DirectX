#include "RailCameraController.h"
void RailCameraController::Initialize(const Transform& transform) {
	camera_ = Camera::GetInstance();
	transform_ = transform;
	camera_->SetPosition({0.0f,0.0f,-30.0f});
}

void RailCameraController::Update() {
	ImGui::Begin("Camera");
	ImGui::SliderFloat3("translate",reinterpret_cast<float*>(&transform_.translate),-3.0f,3.0f);
	ImGui::SliderFloat3("rotate",reinterpret_cast<float*>(&transform_.rotate),-3.0f,3.0f);
	ImGui::End();

	transform_.translate = camera_->GetPosition();
	transform_.translate += {0.0f, 0.0f, 0.0f};

	camera_->SetTransform(transform_);
}