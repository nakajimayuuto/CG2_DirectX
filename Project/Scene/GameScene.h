#pragma once
#include <list>
#include <vector>
#include "IScene.h"
#include "../Satlib.h"
#include "../Skydome.h"
#include "../Ground.h"
#include "../Player.h"
#include "../Boss.h"
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

	void CreateUI(uint32_t xIndex,uint32_t yIndex,std::string name);
private:
	std::unique_ptr<Player> player_;
	std::unique_ptr<Boss> boss_;
	std::unique_ptr<Skydome> skydome_;
	std::unique_ptr<Ground> ground_;

	std::unique_ptr<Emitter> worldFrameEmitter_;
	std::unique_ptr<Emitter> worldBigFrameEmitter_;

	std::unique_ptr<PauseMenu> pauseMenu_;
	std::unique_ptr<GameOverMenu> gameOverMenu_;
	std::unique_ptr<Fade> fade_;
	std::vector<std::pair<std::unique_ptr<Model>, Transform>> uiDatas_;
	std::vector<std::pair<std::unique_ptr<Model>, Transform>> wallDatas_;
	float startFade_;
	bool useSkipStart_;
	bool useSkipEnd_;
private:
	void TutorialInitialize();
	void TutorialUpdate();
	void TutorialDraw();
private:

	float tutorialTimer_;
	float tutorialTimerMax_ = 1.0f;

	float gameclearTimer_;
	float gameclearTimerMax_ = 0.5f;
	float gameclearFadeTimerMax_ = 3.5f;

	float gameoverTimer_;
	float gameoverTimerMax_ = 0.5f;
	float gameoverMenuTimerMax_ = 1.5f;

	bool isBossDeath_;
	bool isBGMStart_;

	bool isClear_;
	bool isGameOver_;

	bool isTutorial_;
	static inline float kTutorialFinishPosX = 105.0f;
private:
	SoundData musPhase1_;
	SoundData musPhase1Intro_;
	SoundData musPhase2_;
	SoundData musPhase2Intro_;
};