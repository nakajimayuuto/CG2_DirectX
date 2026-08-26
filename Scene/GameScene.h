#pragma once
#include <list>
#include "IScene.h"
#include "../Satlib.h"
#include "../Skydome.h"
#include "../Ground.h"
#include "../Player.h"
#include "../GameCamera.h"
#include "../Boss.h"
#include "../Fade.h"
#include "../PauseMenu.h"
class TutorialObject {
public:
	bool isWall_ = true;
	Collider collider_;
	Model model_;
	Transform transform_;
	bool isActive_;
	uint32_t attackCount_;
	uint32_t attackCountMax_;
	float timer_;
	float timerMax_;
	Vector4 color_;

	void Initialize(float timerMax) {
		if (isWall_) {
			isActive_ = true;
			timer_ = 0.0f;
			timerMax_ = timerMax;
			attackCount_ = 1;
			attackCountMax_ = 3;
			collider_.SetSize({ 10.0f,20.0f,1.0f });
			collider_.SetRadius(11.0f);
			collider_.SetColliderType(ColliderType::kBox);
			transform_.Initialize();
			transform_.scale = { 10.0f,20.0f,1.0f };
			transform_.translate = {0.0f,50.0f,0.0f};
			collider_.SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy));
			collider_.SetCollisionMask(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack));
			color_ = { 1.0f,1.0f,1.0f,1.0f };
		} else {

		}
	}

	void CollisionUpdate() {
		if (gGamePhase != GamePhase::kTutorial || !isActive_) {
			return;
		}

		color_ = { 1.0f,1.0f,1.0f ,1.0f};
		if (timer_ < 0.0f) {
			if (collider_.GetOnCollision()) {
				attackCount_++;
				timer_ = timerMax_;
				color_ = { 1.0f,0.0f,0.0f,1.0f };
				SoundManager::GetInstance()->SoundPlay("snd_boss_damage", 1.0f, 0.25f, kSoundEffect);

				if (attackCount_ > attackCountMax_) {
					isActive_ = false;
					SoundManager::GetInstance()->SoundPlay("snd_explode_mini", 1.0f, 0.25f, kSoundEffect);
				}
			}

			collider_.SetOnCollision(false);

			collider_.SetTransform(transform_);
			CollisionManager::GetInstance()->AddColliderList(&collider_);
		} else {
			collider_.SetOnCollision(false);
			timer_ -= DeltaTime::GetInstance()->GetGameTime();
			color_ = { 1.0f,0.0f,0.0f,1.0f };
		}
	}

	void Draw() {
		if (gGamePhase != GamePhase::kTutorial || !isActive_) {
			return;
		}

		Renderer::GetInstance()->DrawBox(transform_, Transform::GetInitialValue({ 10.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), TextureManager::GetInstance()->GetTextureInfo("wall_soul"), color_);
		//model_.Draw(transform_,false);
	}
};

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
	std::unique_ptr<Boss> boss_;

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
	std::unique_ptr<TutorialObject> tutorialAttackWall_;
	std::unique_ptr<TutorialObject> tutorialExitWall_;

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