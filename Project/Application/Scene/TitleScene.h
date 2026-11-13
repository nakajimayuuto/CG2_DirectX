#pragma once
#include "Satlib.h"
#include "IScene.h"
#include "../Ground.h"
#include "../Fade.h"

enum class TitlePhase {
	kTitle,
	kMenuSelect,
	kDifficultySelect,
	kSetting,
	kCount
};

class TitleScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	std::unique_ptr<Fade> fade_;
};