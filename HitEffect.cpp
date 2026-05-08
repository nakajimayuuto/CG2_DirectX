#include "HitEffect.h"

HitEffect* HitEffect::Create(Vector3 position){
	HitEffect* instance = new HitEffect();
	assert(instance);

	instance->Initialize(position);

	return instance;
}

void HitEffect::Initialize(Vector3 position){
	ModelInfo effectInfo = ModelManager::GetInstance()->GetModelInfo("hit_effect_plane");
	model_.Initialize(effectInfo);
	model_.SetLightingType(Renderer::LightingType::kNone);
	transformCircle_.Initialize();
	transformCircle_.translate = position;

	for (Renderer::Model& model : ellipseModels_) {
		model.Initialize(effectInfo);
		model.SetLightingType(Renderer::LightingType::kNone);
	}

	for (Transform& transform : ellipseTransforms_) {
		transform.Initialize();
		transform.scale = {kEllipseWidth,kEllipseHeight,1.0f};
		transform.rotate = { 0.0f,0.0f,Radian(Random::GetInstance()->RandomFloat(0.0f,359.0f)) };
		transform.translate = position;
	}

	status_ = Status::kSpread;

	animationParameter_ = 0.0f;

	isDelete_ = false;
}

void HitEffect::Update(){
	animationParameter_ += 1.0f / 60.0f;

	Vector3 scale = { 0.0f,0.0f,0.0f};
	float rotateSpeed = 3.0f;

	switch (status_){
	case HitEffect::Status::kSpread:
		animationParameter_ = std::min(animationParameter_,kAnimationParameterSpread);

		scale.x = Easing(0.0f,1.0f,animationParameter_,kAnimationParameterSpread,EaseType::kEaseIn);
		scale.y = Easing(0.0f,1.0f,animationParameter_,kAnimationParameterSpread,EaseType::kEaseIn);
		rotateSpeed = Easing(0.0f,3.0f,animationParameter_,kAnimationParameterSpread,EaseType::kEaseIn);

		if (animationParameter_ >= kAnimationParameterSpread) {
			status_ = Status::kShrink;
			animationParameter_ = 0.0f;
		}
		break;
	case HitEffect::Status::kShrink:
		animationParameter_ = std::min(animationParameter_,kAnimationParameterShrink);

		scale.x = Easing(1.0f, 0.0f, animationParameter_, kAnimationParameterShrink, EaseType::kEaseOut);
		scale.y = Easing(1.0f, 0.0f, animationParameter_, kAnimationParameterShrink, EaseType::kEaseOut);
		rotateSpeed = Easing(3.0f, 0.0f, animationParameter_, kAnimationParameterShrink, EaseType::kEaseOut);

		if (animationParameter_ >= kAnimationParameterShrink) {
			status_ = Status::kDelete;
			animationParameter_ = 0.0f;
		}
		break;
	case HitEffect::Status::kDelete:
		isDelete_ = true;
		break;
	}

	transformCircle_.scale = scale;

	for (Transform& transform : ellipseTransforms_) {
		transform.scale.x = scale.x * kEllipseWidth;
		transform.scale.y = scale.y * kEllipseHeight;
		transform.rotate.z += Radian(rotateSpeed);
	}
}

void HitEffect::Draw(){
	model_.Draw(transformCircle_);

	for (uint32_t i = 0; i < kEllipseMax; i++) {
		ellipseModels_[i].Draw(ellipseTransforms_[i]);
	}	
}
