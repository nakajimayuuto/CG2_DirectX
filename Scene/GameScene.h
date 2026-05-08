#pragma once
#include "../Satlib.h"
#include "IScene.h"

#include "../Player.h"
#include "../Enemy.h"
#include "../Skydome.h"
#include "../MapChipField.h"
#include "../CameraController.h"
#include "../DeathParticle.h"
#include "../Fade.h"
#include "../HitEffect.h"
#include <vector>

class GameScene : public IScene{
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void CreateHitEffect(Vector3 position);
private:
	void GenerateBlocks();

	void CheckAllCollision();

	void EnemyRemoveCheck();

	void HitEffectRemoveCheck();

	void ChangePhase();

private:
	enum class Phase {
		kFadeIn, // フェードイン.
		kPlay, // ゲームプレイ.
		kDeath, // デス演出.
		kFadeOut, // フェードアウト.
	};

	// 要素数.
	uint32_t kNumBlockVertical = 20;
	uint32_t kNumBlockHorizontal = 10;
	// ブロック1個分の横幅.
	const float kBlockWidth = 1.0f;
	const float kBlockHeight = 1.0f;

	std::vector<std::vector<Transform*>> transformBlocks_;
	std::vector<std::vector<Renderer::ModelBox*>> modelBlocks_;

	Player* player_ = nullptr;

	static inline const uint32_t kEnemyMax = 3;

	std::list<Enemy*> enemies_;

	std::list<HitEffect*> hitEffects_;

	Phase phase_;

	Skydome* skydome_ = nullptr;

	MapChipField* mapChipField_ = nullptr;

	CameraController* cameraController_ = nullptr;

	DeathParticle* deathParticle_ = nullptr;

	Fade* fade_ = nullptr;
};

