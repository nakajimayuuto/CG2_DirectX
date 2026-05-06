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

	delete skydome_;
	delete mapChipField_;
	delete cameraController_;
}

void GameScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player", "player.obj");
	ModelManager::GetInstance()->RegisterObj("enemy", "Resource/enemy", "enemy.obj");
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");

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

	skydome_ = new Skydome();
	skydome_->Initialize();

	cameraController_ = new CameraController();
	cameraController_->SetTarget(player_);
	cameraController_->SetMovableArea({ 11.5f,100.0f,6.5f,100.0f });
	cameraController_->Initialize();
	cameraController_->SetMode(CameraController::Mode::kFollow);

	GenerateBlocks();
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

void GameScene::CheckAllCollision(){
	for (Enemy* enemy : enemies_) {
		if (Collision::AABBToAABB(player_->GetAABB(),enemy->GetAABB())) {
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}

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

	player_->Update();

	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}

	skydome_->Update();

	cameraController_->Update();

	CheckAllCollision();
}

void GameScene::Draw() {
	player_->Draw();
	
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	skydome_->Draw();

	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (!modelBlocks_[i][j]) {
				continue;
			}

			modelBlocks_[i][j]->Draw(*transformBlocks_[i][j]);
		}
	}
}
