#include "DeltaTime.h"

void DeltaTime::Initialize() {
	frameTime = clock();
	preFrameTime = clock();
}

void DeltaTime::Update() {
	preFrameTime = frameTime;
	frameTime = clock();

	deltaTime = static_cast<float>(frameTime - preFrameTime) * 0.001f;
}