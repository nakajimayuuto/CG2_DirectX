#pragma once
#include "Satlib.h"
#include <assert.h>
#include <array>
class HitEffect{
public:
	static HitEffect* Create(Vector3 position);

	void Initialize(Vector3 position);

	void Update();

	void Draw();

	bool GetIsDelete() { return isDelete_; };

	static void SetModel(Renderer::Model* model) { model_ = model; };
	static void SetCamera(Camera* camera) { camera_ = camera; };
private:
	enum class Status {
		kSpread, // 拡大.
		kShrink, // 縮小.
		kDelete, // 削除
	};
private:
	static Renderer::Model* model_;
	static Camera* camera_;

	Renderer::Model centerModel_;

	Transform transformCircle_;

	// 楕円.
	static inline const uint32_t kEllipseMax = 3;
	static inline const float kEllipseWidth = 2.5f;
	static inline const float kEllipseHeight = 0.1f;

	std::array<Renderer::Model,kEllipseMax> ellipseModels_;

	std::array<Transform,kEllipseMax> ellipseTransforms_;

	// アニメーション.
	Status status_;

	float animationParameter_ = 0.0f;
	static inline const float kAnimationParameterSpread = 0.1f;
	static inline const float kAnimationParameterShrink = 0.4f;

	bool isDelete_ = false;
};

