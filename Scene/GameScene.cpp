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
	delete skydome_;
	delete mapChipField_;
	delete cameraController_;
}

void GameScene::Initialize() {
	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player", "player.obj");
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");

	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(1,18);

	player_ = new Player();
	player_->Initialize(playerPosition);

	skydome_ = new Skydome();
	skydome_->Initialize();

	mapChipField_ = new MapChipField();
	mapChipField_->LoadMapChipCsv("Resource/map/block.csv");

	cameraController_ = new CameraController();
	cameraController_->SetTarget(player_);
	cameraController_->SetMovableArea({11.5f,100.0f,6.5f,100.0f});
	cameraController_->Initialize();

	GenerateBlocks();
}

void GameScene::GenerateBlocks() {
	// 要素数を変更する.
	kNumBlockVirtical = mapChipField_->GetNumBlockVirtical();
	kNumBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	transformBlocks_.resize(kNumBlockVirtical);
	modelBlocks_.resize(kNumBlockVirtical);
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		// 1列の要素数を設定(横方向のブロック数).
		transformBlocks_[i].resize(kNumBlockHorizontal);
		modelBlocks_[i].resize(kNumBlockHorizontal);

	}

	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j,i) == MapChipType::kBlock) {
				modelBlocks_[i][j] = new Renderer::ModelBox();
				modelBlocks_[i][j]->Initialize();

				transformBlocks_[i][j] = new Transform();
				transformBlocks_[i][j]->Initialize();
				transformBlocks_[i][j]->translate = mapChipField_->GetMapChipPositionByIndex(j,i);
			}
		}
	}
}

void GameScene::Update() {
#ifdef _DEBUG
	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}
#endif // _DEBUG

	player_->Update();
	skydome_->Update();

	cameraController_->Update();
}

void GameScene::Draw() {
	player_->Draw();
	skydome_->Draw();

	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (!modelBlocks_[i][j]) {
				continue;
			}

			modelBlocks_[i][j]->Draw(*transformBlocks_[i][j]);
		}
	}
}
