#pragma once
#include "Satlib.h"
#include "BaseEffect.h"
#include <assert.h>
#include <array>

class HitEffect final : public BaseEffect{
public:
	void Initialize(Vector3 position) override;

	void Update() override;

	void Draw() override;
private:
	enum class Status {
		kSpread, // 拡大.
		kShrink, // 縮小.
		kDelete, // 削除
	};
private:
	// 中心円.
	Renderer::Model model_;

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
};

