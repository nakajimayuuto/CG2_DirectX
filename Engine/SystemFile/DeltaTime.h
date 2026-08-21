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
	float GetGameTime() const { return gameTime; };

	float GetDeltaTimePerFrame() const { return deltaTime * 60.0f; };

	void GetStartDebugTime() { debugDeltaTime = static_cast<float>(frameDebugTime - preFrameDebugTime) * 0.001f; preFrameDebugTime = clock(); };

	void GetEndDebugTime() { frameDebugTime = clock(); };

	void SetGameTimeSpeed(float speed) { gameTimeSpeed_ = speed; };

	float GetGameTimeSpeed() { return gameTimeSpeed_; };

	void SetHitStop(float stopTime) { 
		stopTimer_ = stopTime; isHitStop_ = true; 
	};
private:
	void DebugUpdate();
private:
	// FPS系
	clock_t frameTime;
	clock_t preFrameTime;
	clock_t frameDebugTime;
	clock_t preFrameDebugTime;
	float deltaTime;
	float debugDeltaTime;

	// DeltaTime
	float gameTime; // プレイヤーや弾の動き等を司る.ヒットストップで止まる値.
	float gameTimeSpeed_;
	float particleTime; // パーティクルや一部演出に使用.ヒットストップで止まらない.
	float applicationTime; // Pauseメニューやらのパーティクルが止まっても止まらない値に利用.

	// ヒットストップ.
	float stopTimer_;
	bool isHitStop_;
	

};

