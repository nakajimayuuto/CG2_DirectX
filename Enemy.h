#pragma once
#include "Satlib.h"
class Player;

class Enemy{
public:
	void Initialize(const Vector3& position);

	void Update();

	void Draw();

	Vector3	GetWorldPosition();

	AABB GetAABB();

	void OnCollision(const Player* player);
private:
	void WalkAnimationUpdate();
private:
	// 移動.
	static inline const float kWalkSpeed = 0.01f;

	Vector3 velocity_ = {};

	// 当たり判定.
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

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