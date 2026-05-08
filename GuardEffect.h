#pragma once
#include "Satlib.h"
#include "BaseEffect.h"
#include <assert.h>
#include <array>

class GuardEffect final : public BaseEffect{
public:
	void Initialize(Vector3 position) override;

	void Update() override;

	void Draw() override;
private:
	enum class Status {
		kSpread, // 拡大.
		kFadeOut, // フェードアウト.
		kDelete, // 削除
	};
private:
	// 中心円.
	Renderer::Model model_;

	Transform transform_;

	// アニメーション.
	Status status_;

	float animationParameter_ = 0.0f;
	static inline const float kAnimationParameterSpread = 0.5f;
	static inline const float kAnimationParameterFadeOut = 0.5f;
};

