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

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	enum class Status {
		kSpread, // 拡大.
		kShrink, // 縮小.
		kDelete, // 削除
	};
private:
	// 中心円.
	Renderer::Model model_;

	Transform transform_;

	// 楕円.
	static inline const uint32_t kEllipseMax = 3;
	static inline float kEllipseWidth = 2.5f;
	static inline float kEllipseHeight = 0.1f;

	std::array<Renderer::Model,kEllipseMax> ellipseModels_;

	std::array<Transform,kEllipseMax> ellipseTransforms_;

	// アニメーション.
	Status status_;

	float animationParameter_ = 0.0f;
	static inline float kAnimationParameterSpread = 0.1f;
	static inline float kAnimationParameterShrink = 0.4f;
};

