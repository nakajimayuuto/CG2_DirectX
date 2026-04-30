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
private:
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

	// 旋回制御.
	static inline const float kTimeTurn = 0.3f;

	LRDirection lrDirection_ = LRDirection::kRight;
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	// 描画.
	Renderer::Model model_;
};

