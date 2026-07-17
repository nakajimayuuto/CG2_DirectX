#pragma once
#include "Satlib.h"
class HPGauge{
public:
	void Initialize(float* currentHP , float maxHP,const Vector2& size);

	void Update();

	void Draw();

	void SetPosition(const Vector2& position) { transform_.translate = position; };

	void SetScale(const Vector2& scale) { transform_.scale = scale; };

	void SetAlpha(float alpha) { alpha_ = alpha; };
private:
	float* targetHP_;
	float maxHP_;
	static inline Vector2 kBlank = {5.0f,5.0f};
	Vector2 size_;

	float alpha_;

	Transform2D transform_;
};

