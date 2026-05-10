#pragma once
#include "Satlib.h"

class Player;

class CameraController{
public:
	enum class Mode {
		kFollow, // プレイヤー追従.
		kForcedScroll, // 強制スクロール.
	};

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

	void SetMode(Mode mode) { mode_ = mode; };

	Mode GetMode() { return mode_; }

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	void FollowUpdate();
	void ForcedScrollUpdate();
private:
	// 補間.
	static inline float kInterpolationRate = 0.3f;

	Vector3 afterPosition_ = {0.0f,0.0f,0.0f};

	// 加減速.
	static inline float kVelocityBias = 4.0f;

	static inline Rect kMargin = { -10.0f,10.0f,-10.0f,10.0f };

	// モード.
	Mode mode_ = Mode::kFollow;

	Vector3 cameraPosition_;

	float kCameraEndBlank_ = 12.0f;

	Vector3 velocity_ = {0.02f,0.0f,0.0f};

	Player* target_ = nullptr;

	Rect movableArea_ = { 0.0f,100.0f,0.0f,100.0f };

	Vector3 targetOffset_ = {0.0f,0.0f,-30.0f};
};

