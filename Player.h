#pragma once
#include "Satlib.h"
/// <summary>
/// 自キャラ
/// </summary>
class Player{
public:
	void Initialize(const Vector3& position);

	void Update();

	void Draw();

	Transform GetTransform() { return transform_; };

	Vector3 GetVelocity() { return velocity_; };
private:
	void MovingUpdate();

	void TurningControl();
private:
	enum class LRDirection {
		kRight,
		kLeft,
	};

	// 移動.
	static inline const float kAcceletation = 0.02f;
	static inline const float kAttenuation = 0.25f;
	static inline const float kLimitRunSpeed = 1.0f;

	Transform transform_;
	Vector3 velocity_ = {};

	// ジャンプ.
	static inline const float kGravityAcceleration = 0.098f;
	static inline const float kLimitFallSpeed = 1.0f;
	static inline const float kJumpAcceleration = 1.0f;

	bool onGround_ = true;

	// 旋回制御.
	static inline const float kTimeTurn = 0.3f;

	LRDirection lrDirection_ = LRDirection::kRight;
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	// 描画.
	Renderer::Model model_;
};

