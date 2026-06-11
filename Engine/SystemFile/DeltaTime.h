#pragma once
#include <windows.h>
#include <time.h>

class DeltaTime{
public:
	static DeltaTime* GetInstance();

	DeltaTime() { Initialize(); };

	void Initialize();

	void Update();

	float GetDeltaTime() const { return deltaTime; };

	float GetDeltaTimePerFrame() const { return deltaTime * 60.0f; };
private:
	void DebugUpdate();
private:
	clock_t frameTime;
	clock_t preFrameTime;


	float deltaTime;
};

