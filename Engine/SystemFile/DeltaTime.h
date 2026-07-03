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

	void GetStartDebugTime() { debugDeltaTime = static_cast<float>(frameDebugTime - preFrameDebugTime) * 0.001f; preFrameDebugTime = clock(); };

	void GetEndDebugTime() { frameDebugTime = clock(); };
private:
	void DebugUpdate();
private:
	clock_t frameTime;
	clock_t preFrameTime;

	clock_t frameDebugTime;
	clock_t preFrameDebugTime;


	float deltaTime;
	float debugDeltaTime;
};

