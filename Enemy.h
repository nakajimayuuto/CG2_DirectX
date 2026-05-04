#pragma once
#include "Satlib.h"
class Enemy{
public:
	void Initialize(const Vector3& position);

	void Update();

	void Draw();
private:
	void WalkAnimationUpdate();
private:
	// 移動.
	static inline const float kWalkSpeed = 0.01f;

	Vector3 velocity_ = {};

	// アニメーション.
	// 最初の角度.
	static inline const float kWalkMotionAngleStart = -15.0f;
	// 最後の角度.
	static inline const float kWalkMotionAngleEnd = 15.0f;
	// アニメーションの周期	(秒).
	static inline const float kWalkMotionTime = 1.0f;

	float walkTimer_ = 0.0f;



	Renderer::Model model_;

	Transform transform_;

};