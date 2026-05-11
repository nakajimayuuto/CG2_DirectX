#include "GuardEffect.h"
void GuardEffect::Initialize(Vector3 position){
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("hit_effect_plane"));
	model_.SetLightingType(Renderer::LightingType::kNone);
	model_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("guard_effect_plane"));
	transform_.Initialize();
	transform_.translate = position;
	status_ = Status::kSpread;

	animationParameter_ = 0.0f;

	transform_.scale = {0.0f,0.0f,1.0f};

	isDelete_ = false;
}

void GuardEffect::Update(){
	animationParameter_ += 1.0f / 60.0f;

	Vector3 scaleSpeed = { 0.001f,0.001f,0.0f};
	Vector4 color = {1.0f,1.0f,1.0f,1.0f};

	switch (status_){
	case GuardEffect::Status::kSpread:
		animationParameter_ = std::min(animationParameter_,kAnimationParameterSpread);

		scaleSpeed.x = Easing(0.1f,0.001f,animationParameter_,kAnimationParameterSpread,EaseType::kEaseIn);
		scaleSpeed.y = Easing(0.1f,0.001f,animationParameter_,kAnimationParameterSpread,EaseType::kEaseIn);

		if (animationParameter_ >= kAnimationParameterSpread) {
			status_ = Status::kFadeOut;
			animationParameter_ = 0.0f;
		}
		break;
	case GuardEffect::Status::kFadeOut:
		animationParameter_ = std::min(animationParameter_,kAnimationParameterFadeOut);

		color = { 1.0f,1.0f,1.0f,Easing(10.0f,0.01f,animationParameter_,kAnimationParameterFadeOut,EaseType::kEaseOut) };

		model_.SetColor(color);

		if (animationParameter_ >= kAnimationParameterFadeOut) {
			status_ = Status::kDelete;
			animationParameter_ = 0.0f;
		}
		break;
	case GuardEffect::Status::kDelete:
		isDelete_ = true;
		break;
	}

	transform_.scale += scaleSpeed;
}

void GuardEffect::Draw(){
	model_.Draw(transform_);
}

void GuardEffect::RegisterGlobalVariables() {

	const char* groupName = "GuardEffect";

	GlobalVariables::GetInstance()->CreateGroup(groupName);

	GlobalVariables::GetInstance()->AddItem(groupName, "AnimationParameterSpread", kAnimationParameterSpread);
	GlobalVariables::GetInstance()->AddItem(groupName, "AnimationParameterFadeOut", kAnimationParameterFadeOut);
}

void GuardEffect::ApplyGlobalVariables() {
	const char* groupName = "GuardEffect";

	kAnimationParameterSpread = GlobalVariables::GetInstance()->GetFloatValue(groupName, "AnimationParameterSpread");
	kAnimationParameterFadeOut = GlobalVariables::GetInstance()->GetFloatValue(groupName, "AnimationParameterFadeOut");
}
