#include "HPGauge.h"

void HPGauge::Initialize(float* currentHP, float maxHP, const Vector2& size) {
	targetHP_ = currentHP;
	maxHP_ = maxHP;
	transform_.Initialize();
	size_ = size;

	alpha_ = 1.0f;
}

void HPGauge::Update() {

}

void HPGauge::Draw() {
	Vector2 newSize = size_;
	newSize.x = newSize.x + kBlank.x;
	newSize.y = newSize.y + kBlank.y;
	Vector2 newNewSize = size_;

	newNewSize.x = size_.x * Easing(0.0f,1.0f,*targetHP_,maxHP_,EaseType::kConstant);
	Transform2D newTransform;
	newTransform = transform_;
	newTransform.translate.x = newTransform.translate.x - size_.x * Easing(0.5f, 0.0f, *targetHP_, maxHP_, EaseType::kConstant);

	Renderer::GetInstance()->DrawSprite(transform_.GetTransformValue(), newSize, "white_template", {0.0f,0.0f,0.0f,alpha_});
	Renderer::GetInstance()->DrawSprite(transform_.GetTransformValue(), size_, "white_template", {1.0f,0.1f,0.1f,alpha_ });
	Renderer::GetInstance()->DrawSprite(newTransform.GetTransformValue(), newNewSize, "white_template", { 1.0f,1.0f,0.1f,alpha_ });
}