#include "HPGauge.h"

void HPGauge::Initialize(float* currentHP, float maxHP, const Vector2& size) {
	targetHP_ = currentHP;
	maxHP_ = maxHP;
	transform_.Initialize();
	size_ = size;

	alpha_ = 1.0f;

	color_ = { 1.0f,1.0f,0.1f };
	backColor_ = { 1.0f, 0.1f, 0.1f };
	isActiveLine_ = false;
	lineInterval_ = 10;
}

void HPGauge::Update() {

}

void HPGauge::Draw() {
	Vector2 newSize = size_;
	newSize.x = newSize.x + kBlank.x;
	newSize.y = newSize.y + kBlank.y;
	Vector2 newNewSize = size_;

	newNewSize.x = size_.x * Easing(0.0f, 1.0f, *targetHP_, maxHP_, EaseType::kConstant);
	Transform2D newTransform;
	newTransform = transform_;
	newTransform.translate.x = newTransform.translate.x - (size_.x * Easing(0.5f, 0.0f, *targetHP_, maxHP_, EaseType::kConstant) * transform_.scale.x);

	Renderer::GetInstance()->DrawSprite(transform_.GetTransformValue(), newSize, "white_template", { 0.0f,0.0f,0.0f,alpha_ });
	Renderer::GetInstance()->DrawSprite(transform_.GetTransformValue(), size_, "white_template", { backColor_.x,backColor_.y,backColor_.z,alpha_ });
	Renderer::GetInstance()->DrawSprite(newTransform.GetTransformValue(), newNewSize, "white_template", { color_.x,color_.y,color_.z,  alpha_ });
	if (isActiveLine_) {
		Transform2D newLineTransform;
		for (uint32_t i = 1; i < static_cast<uint32_t>(maxHP_ / static_cast<float>(lineInterval_)); i++) {
			newLineTransform = transform_;
			newLineTransform.translate.x = transform_.translate.x + i * (size_.x / (maxHP_ / static_cast<float>(lineInterval_))) - (size_.x / 2.0f) + (kBlank.x / 4.0f);
			Renderer::GetInstance()->DrawSprite(newLineTransform.GetTransformValue(), { kBlank.x / 2.0f,newSize.y }, "white_template", { 0.0f,0.0f,0.0f,alpha_ });
		}
	}
}