#pragma once
#include "../Satlib.h"
#include "IScene.h"
#include "../Fade.h"

class TitleScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	enum class Phase {
		kFadeIn, // フェードイン.
		kMain, // メイン部.
		kFadeOut, // フェードアウト.
	};

	Renderer::Model titleModel_;

	Transform titleTransform_;

	Renderer::Model playerModel_;

	Transform playerTransform_;

	Fade* fade_ = nullptr;

	Phase phase_ = Phase::kFadeIn;
};