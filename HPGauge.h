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

	void SetColor(const Vector3& color) { color_ = color; };
	void SetBackColor(const Vector3& color) { backColor_ = color; };
private:
	float* targetHP_;
	float maxHP_;
	static inline Vector2 kBlank = {5.0f,5.0f};
	Vector2 size_;

	float alpha_;

	Vector3 color_;
	Vector3 backColor_;

	Transform2D transform_;
};

