#include "DeltaTime.h"

DeltaTime* DeltaTime::GetInstance() {
	static DeltaTime instance;
	return &instance;
}

void DeltaTime::Initialize() {
	frameTime = clock();
	preFrameTime = clock();
}

void DeltaTime::Update() {
	preFrameTime = frameTime;
	frameTime = clock();

	deltaTime = static_cast<float>(frameTime - preFrameTime) * 0.001f;
}