#include "GameScene.h"
#include "../Satlib.h"
#include "../Enemy.h"
#include "../ShieldEnemy.h"
#include "../Managers/StageManager.h"

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

	for (BaseEnemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	for (BaseEffect* hitEffect : hitEffects_) {
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
	ModelManager::GetInstance()->RegisterObj("shield_enemy", "Resource/shield_enemy", "shield_enemy.obj");
	ModelManager::GetInstance()->RegisterObj("plane", "Resource", "plane.obj");
	ModelManager::GetInstance()->RegisterObj("hit_effect_plane", "Resource/HitEffectPlane", "hit_effect_plane.obj");

	ModelManager::GetInstance()->RegisterObj("death_particle", "Resource/death_particle", "death_particle.obj");
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("player_attack_effect", "Resource/player/player_attack_effect.png");
	TextureManager::GetInstance()->RegisterTexture("guard_effect_plane", "Resource/HitEffectPlane/guard_effect_plane.png");

	SoundManager::GetInstance()->RegisterSound("free_k", "Resource/free_k.wav");

	CreateStage();

	GenerateFieldObjects();

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

	phase_ = Phase::kFadeIn;
}

void GameScene::CreateStage(){

	const StageData& stageData = StageManager::GetInstance()->GetCurrentStageData();	

	std::string stageFileName = "Resource/map/" + stageData.name + ".csv";

	mapChipField_ = new MapChipField();

	mapChipField_->LoadMapChipCsv(stageFileName);
}

void GameScene::GenerateFieldObjects() {
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
	
	BaseEnemy* newEnemy;

	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			switch (mapChipField_->GetMapChipTypeByIndex(j, i)){
			case MapChipType::kBlock:
				modelBlocks_[i][j] = new Renderer::ModelBox();
				modelBlocks_[i][j]->Initialize();

				transformBlocks_[i][j] = new Transform();
				transformBlocks_[i][j]->Initialize();
				transformBlocks_[i][j]->translate = mapChipField_->GetMapChipPositionByIndex(j, i);
				break;
			case MapChipType::kPlayer:
				assert(player_ == nullptr && "自キャラを二重に配置しようとしています");
				player_ = new Player();
				player_->Initialize(mapChipField_->GetMapChipPositionByIndex(j, i));
				player_->SetMapChipField(mapChipField_);
				break;
			case MapChipType::kEnemy:
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j,i);
				switch (subID){
				case 0:
					newEnemy = new Enemy();
					newEnemy->Initialize(mapChipField_->GetMapChipPositionByIndex(j, i));

					enemies_.push_back(newEnemy);
					break;
				case 1:
					newEnemy = new ShieldEnemy();
					newEnemy->Initialize(mapChipField_->GetMapChipPositionByIndex(j, i));

					enemies_.push_back(newEnemy);
					break;
				}
				break;
			}
		}
	}
}

void GameScene::CheckAllCollision() {
	for (BaseEnemy* enemy : enemies_) {
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
	enemies_.remove_if([](BaseEnemy* enemy) {
		if (enemy->GetIsDead()) {
			delete enemy;
			return true;
		}
		return false;
		});
}

void GameScene::HitEffectRemoveCheck(){
	hitEffects_.remove_if([](BaseEffect* hitEffect_) {
		if (hitEffect_->GetIsDelete()) {
			delete hitEffect_;
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
		SceneManager::GetInstance()->ReloadScene();
	}
#endif // _DEBUG

	switch (phase_) {
	case GameScene::Phase::kFadeIn:
		skydome_->Update();

		for (BaseEnemy* enemy : enemies_) {
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

		for (BaseEnemy* enemy : enemies_) {
			enemy->Update();
		}

		for (BaseEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		cameraController_->Update();

		ChangePhase();

		CheckAllCollision();

		EnemyRemoveCheck();

		HitEffectRemoveCheck();
		break;
	case GameScene::Phase::kDeath:
		skydome_->Update();

		deathParticle_->Update();

		for (BaseEnemy* enemy : enemies_) {
			enemy->Update();
		}

		for (BaseEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		HitEffectRemoveCheck();

		Camera::GetInstance()->Update();

		if (deathParticle_ && deathParticle_->GetIsFinished()) {

			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
		}
		break;
	case GameScene::Phase::kFadeOut:
		skydome_->Update();

		deathParticle_->Update();

		for (BaseEnemy* enemy : enemies_) {
			enemy->Update();
		}

		for (BaseEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		HitEffectRemoveCheck();

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


	for (BaseEnemy* enemy : enemies_) {
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

	for (BaseEffect* hitEffect : hitEffects_) {
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

void GameScene::CreateEffect(Vector3 position, BaseEffect::EffectType type){
	BaseEffect* newHitEffect = BaseEffect::Create(position,type);
	hitEffects_.push_back(newHitEffect);
}
