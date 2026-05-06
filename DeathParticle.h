#pragma once
#include "satlib.h"
#include <array>

class DeathParticle {
public:
	void Initialize();

	void Update();

	void Draw();

	void Start(const Vector3& position);

	bool GetIsFinished() { return isFinished_; };
private:
	// パーティクルの量.
	static inline const uint32_t kNumParticles = 8;

	std::array<Renderer::Model,kNumParticles> models_;
	std::array<Transform,kNumParticles> transforms_;

	// 消滅時間.
	static inline const float kDuration = 2.0f;
	// 速さ.
	static inline const float kSpeed = 0.05f;
	// 一つ毎の角度.
	static inline const float kAngleUnit = Radian(360.0f / kNumParticles);

	// 終了フラグ.
	bool isFinished_ = false;

	float counter_ = 0.0f;

	Vector4 color_;
};