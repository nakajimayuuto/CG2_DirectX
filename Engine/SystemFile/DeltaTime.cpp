#include "DeltaTime.h"
#include "ImGui.h"
#include <format>

DeltaTime* DeltaTime::GetInstance() {
	static DeltaTime instance;
	return &instance;
}

void DeltaTime::Initialize() {
	frameTime = clock();
	preFrameTime = clock();
	stopTimer_ = 0.0f;
	isHitStop_ = false;
}

void DeltaTime::Update() {

	preFrameTime = frameTime;
	frameTime = clock();

	deltaTime = static_cast<float>(frameTime - preFrameTime) * 0.001f;

	applicationTime = deltaTime;
	particleTime = deltaTime;

	if(isHitStop_){
		stopTimer_ -= deltaTime;

		if (stopTimer_ <= 0.0f) {
			stopTimer_ = 0.0f;
			isHitStop_ = false;
		}
		gameTime = 0.0f;
	} else {
		gameTime = deltaTime;
	}



	DebugUpdate();
}

void DeltaTime::DebugUpdate() {
#ifdef _DEBUG
	ImGui::Begin("DeltaTime");
	ImGui::Text(std::format("FPS : {} / 60", static_cast<int>(60.0f / (deltaTime * 60.0f))).c_str());
	ImGui::Text(std::format("inGame : {} / 60", static_cast<int>(60.0f / (gameTime * 60.0f))).c_str());
	ImGui::Text(std::format("DebugFPS : {}", static_cast<int>(60.0f / (debugDeltaTime * 60.0f))).c_str());


	ImGui::End();
#endif // _DEBUG

}
