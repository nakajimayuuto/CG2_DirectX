#include "GameScene.h"
#include "../Satlib.h"

GameScene::~GameScene() {
	for (std::vector<Transform*>& transformBlockLine : transformBlocks_) {
		for (Transform* transformBlock : transformBlockLine) {
			delete transformBlock;
		}
	}
	transformBlocks_.clear();

	for (std::vector<Renderer::ModelBox*>& modelBlockLine : modelBlocks_) {
		for (Renderer::ModelBox* modelBlock : modelBlockLine) {
			delete modelBlock;
		}
	}
	modelBlocks_.clear();

	delete player_;

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	for (HitEffect* hitEffect : hitEffects_) {
		delete hitEffect;
	}
	hitEffects_.clear();

	delete skydome_;
	delete mapChipField_;
	delete cameraController_;
	delete deathParticle_;
}

void GameScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player", "player.obj");
	ModelManager::GetInstance()->RegisterObj("enemy", "Resource/enemy", "enemy.obj");
	ModelManager::GetInstance()->RegisterObj("plane", "Resource", "plane.obj");
	ModelManager::GetInstance()->RegisterObj("hit_effect_plane", "Resource/HitEffectPlane", "hit_effect_plane.obj");

	ModelManager::GetInstance()->RegisterObj("death_particle", "Resource/death_particle", "death_particle.obj");
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("player_attack_effect", "Resource/player/player_attack_effect.png");

	SoundManager::GetInstance()->RegisterSound("free_k", "Resource/free_k.wav");

	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1, 18);

	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resource/map/block.csv");

	player_ = new Player();
	player_->Initialize(playerPosition);
	player_->SetMapChipField(mapChipField_);

	enemies_.clear();
	for (int32_t i = 0; i < kEnemyMax; i++) {
		Enemy* newEnemy_ = new Enemy();
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(12 + (2 * i), 18 - i);
		newEnemy_->Initialize(enemyPosition);

		enemies_.push_back(newEnemy_);
	}

	hitEffects_.clear();

	skydome_ = new Skydome();
	skydome_->Initialize();

	deathParticle_ = new DeathParticle;
	deathParticle_->Initialize();

	Camera::GetInstance()->Initialize();
	cameraController_ = new CameraController();
	cameraController_->SetTarget(player_);
	cameraController_->SetMovableArea({ 11.5f,100.0f,6.5f,100.0f });
	cameraController_->Initialize();
	cameraController_->SetMode(CameraController::Mode::kFollow);

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	GenerateBlocks();

	phase_ = Phase::kFadeIn;
}

void GameScene::GenerateBlocks() {
	// 要素数を変更する.
	kNumBlockVertical = mapChipField_->GetNumBlockVirtical();
	kNumBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	transformBlocks_.resize(kNumBlockVertical);
	modelBlocks_.resize(kNumBlockVertical);
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		// 1列の要素数を設定(横方向のブロック数).
		transformBlocks_[i].resize(kNumBlockHorizontal);
		modelBlocks_[i].resize(kNumBlockHorizontal);

	}

	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				modelBlocks_[i][j] = new Renderer::ModelBox();
				modelBlocks_[i][j]->Initialize();

				transformBlocks_[i][j] = new Transform();
				transformBlocks_[i][j]->Initialize();
				transformBlocks_[i][j]->translate = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

void GameScene::CheckAllCollision() {
	for (Enemy* enemy : enemies_) {
		if (enemy->GetIsCollisionDisable()) {
			continue;
		}

		if (Collision::AABBToAABB(player_->GetAABB(), enemy->GetAABB())) {
			player_->OnCollision(enemy);
			enemy->OnCollision(this,player_);
		}
	}

}

void GameScene::EnemyRemoveCheck() {
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->GetIsDead()) {
			delete enemy;
			return true;
		}
		return false;
		});
}

void GameScene::Update() {
#ifdef _DEBUG
	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		Initialize();
	}
#endif // _DEBUG

	switch (phase_) {
	case GameScene::Phase::kFadeIn:
		skydome_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		fade_->Update();

		if (fade_->GetIsFinished()) {
			phase_ = Phase::kPlay;
		}

		cameraController_->Update();
		break;
	case GameScene::Phase::kPlay:
		skydome_->Update();

		player_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		cameraController_->Update();

		ChangePhase();

		CheckAllCollision();

		EnemyRemoveCheck();
		break;
	case GameScene::Phase::kDeath:
		skydome_->Update();

		deathParticle_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		Camera::GetInstance()->Update();

		if (deathParticle_ && deathParticle_->GetIsFinished()) {

			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
		}
		break;
	case GameScene::Phase::kFadeOut:
		skydome_->Update();

		deathParticle_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		if (fade_->GetIsFinished()) {
			SceneManager::GetInstance()->ChengeScene(SceneName::kTitleScene);
		}

		fade_->Update();
		Camera::GetInstance()->Update();
		break;
	}
}

void GameScene::Draw() {
	skydome_->Draw();


	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (!modelBlocks_[i][j]) {
				continue;
			}

			modelBlocks_[i][j]->Draw(*transformBlocks_[i][j]);
		}
	}

	player_->Draw();

	deathParticle_->Draw();

	for (HitEffect* hitEffect : hitEffects_) {
		hitEffect->Draw();
	}

	fade_->Draw();
}

void GameScene::ChangePhase() {
	if (!player_->GetIsDead()) {
		return;
	}

	Vector3 deathParticlePosition;

	switch (phase_) {
	case GameScene::Phase::kPlay:
		phase_ = Phase::kDeath;

		deathParticlePosition = player_->GetTransform().translate;

		deathParticle_->Start(deathParticlePosition);
		break;
	case GameScene::Phase::kDeath:
		break;
	}
}

void GameScene::CreateHitEffect(Vector3 position){
	HitEffect* newHitEffect = HitEffect::Create(position);
	hitEffects_.push_back(newHitEffect);
}
