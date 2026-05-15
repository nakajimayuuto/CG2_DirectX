#include "RailCameraController.h"
void RailCameraController::Initialize(const Transform& transform) {
	camera_ = Camera::GetInstance();
	transform_ = transform;
	camera_->SetPosition({0.0f,0.0f,-30.0f});

	//controlPoints_ = {
	//	{0.0f,50.0f,0.0f},
	//	{0.0f,50.0f,10.0f},
	//	{0.0f,50.0f,20.0f},
	//	{0.0f,60.0f,30.0f},
	//	{0.0f,65.0f,40.0f},
	//	{0.0f,60.0f,50.0f},
	//	{0.0f,50.0f,60.0f},
	//	{0.0f,50.0f,70.0f}
	//};

	controlPoints_ = {
		{0.0f,50.0f,0.0f},
		{0.0f,50.0f,10.0f},
		{0.0f,50.0f,20.0f},
		{0.0f,60.0f,30.0f},
		{0.0f,60.0f,40.0f},
		{0.0f,70.0f,50.0f},
		{0.0f,70.0f,60.0f},
		{0.0f,80.0f,70.0f},
		{0.0f,90.0f,80.0f},
		{0.0f,100.0f,80.0f},
		{0.0f,110.0f,80.0f},
		{0.0f,110.0f,80.0f},
		{0.0f,120.0f,90.0f},
		{0.0f,120.0f,100.0f},
		{0.0f,120.0f,110.0f},
		{0.0f,120.0f,120.0f},
		{0.0f,120.0f,130.0f},
		{0.0f,120.0f,140.0f},
		{0.0f,120.0f,150.0f},
	};
}

void RailCameraController::Update() {
	ImGui::Begin("Camera");
	ImGui::SliderFloat3("translate", reinterpret_cast<float*>(&transform_.translate), -6.0f, 6.0f);
	ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&transform_.rotate), 0.01f, -6.0f, 6.0f);
	ImGui::End();

	timer_ += 1.0f / 60.0f;

	transform_.translate = CatmullRomInterpolation(controlPoints_, timer_, timeMax);
	Vector3 target = CatmullRomInterpolation(controlPoints_, timer_+ (1.0f / 60.0f), timeMax);

	target = target - transform_.translate;
	
	transform_.rotate.y = std::atan2(target.x, target.z);
	Vector3 velocityXZ = { target.x,0.0f,target.z };
	transform_.rotate.x = std::atan2(-target.y, velocityXZ.Length());

	camera_->SetTransform(transform_);
}

void RailCameraController::Draw() {
	std::vector<Vector3>pointsDrawing;
	const size_t segmentCount = 100;

	for (size_t i = 0; i < segmentCount; i++) {
		float t = 1.0f / segmentCount * i;
		Vector3 pos = CatmullRomInterpolation(controlPoints_, t, 1.0f);
		pointsDrawing.push_back(pos);
	}

	for (size_t i = 0; i < pointsDrawing.size() - 1; i++) {
		Renderer::Line::GetInstance()->Draw(pointsDrawing[i], pointsDrawing[i + 1], { 1.0f,1.0f,1.0f,1.0f });
	}
}