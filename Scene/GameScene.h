#pragma once
#include <list>
#include "IScene.h"
#include "../Satlib.h"
#include "../Skydome.h"
#include "../Ground.h"
#include "../Player.h"
#include "../GameCamera.h"
#include "../Fade.h"
#include "../PauseMenu.h"
#include "../MapChipManager.h"

class GameScene : public IScene {
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	void CheckAllCollisions();

	void AnimSkipUpdate();
	void AnimSkipFadeUpdate();
private:
	std::unique_ptr<Player> player_;
	std::unique_ptr<Skydome> skydome_;
	std::unique_ptr<Ground> ground_;

	std::unique_ptr<Emitter> worldFrameEmitter_;
	std::unique_ptr<Emitter> worldBigFrameEmitter_;

	std::unique_ptr<PauseMenu> pauseMenu_;
	std::unique_ptr<GameOverMenu> gameOverMenu_;
	std::unique_ptr<Fade> fade_;
	float startFade_;
	bool useSkipStart_;
	bool useSkipEnd_;
private:
	void TutorialInitialize();
	void TutorialUpdate();
	void TutorialDraw();
private:
	int type = 0.0f;
	int count = 0.0f;
	float direction = 15.0f;

	float tutorialTimer_;
	float tutorialTimerMax_ = 1.0f;

	float gameclearTimer_;
	float gameclearTimerMax_ = 0.5f;

	float gameoverTimer_;
	float gameoverTimerMax_ = 0.5f;
	float gameoverMenuTimerMax_ = 1.5f;

	bool isBossDeath_;
	bool isBGMStart_;
private:
	SoundData musPhase1_;
	SoundData musPhase1Intro_;
	SoundData musPhase2_;
	SoundData musPhase2Intro_;
};