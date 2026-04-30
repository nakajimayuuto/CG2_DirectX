#pragma once
#include "Satlib.h"

class Player;

class CameraController{
public:
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float botom = 0.0f;
		float top = 1.0f;
	};

	void Initialize();
	void Update();

	void SetTarget(Player* target) { target_ = target;};

	void SetMovableArea(Rect area) { movableArea_ = area; };

	void Reset();
private:
	// 補間.
	static inline const float kInterpolationRate = 0.3f;

	Vector3 afterPosition_ = {0.0f,0.0f,0.0f};

	// 加減速.
	static inline const float kVelocityBias = 4.0f;

	static inline const Rect kMargin = { -10.0f,10.0f,-10.0f,10.0f };

	Player* target_ = nullptr;

	Rect movableArea_ = { 0.0f,100.0f,0.0f,100.0f };

	Vector3 targetOffset_ = {0.0f,0.0f,-30.0f};
};

