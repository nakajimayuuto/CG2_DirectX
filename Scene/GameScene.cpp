#include "GameScene.h"
#include "../Satlib.h"

GameScene::~GameScene(){
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
}

void GameScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker","Resource/uvChecker.png");

	player_ = new Player();
	player_->Initialize();

	// 要素数を変更する.
	transformBlocks_.resize(kNumBlockVirtical);
	modelBlocks_.resize(kNumBlockVirtical);
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		// 1列の要素数を設定(横方向のブロック数).
		transformBlocks_[i].resize(kNumBlockHorizontal);
		modelBlocks_[i].resize(kNumBlockHorizontal);
		
	}

	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			modelBlocks_[i][j] = new Renderer::ModelBox();
			modelBlocks_[i][j]->Initialize();


			transformBlocks_[i][j] = new Transform();
			transformBlocks_[i][j]->Initialize();
			transformBlocks_[i][j]->translate.x = kBlockWidth * i;
			transformBlocks_[i][j]->translate.y = kBlockHeight * j;
		}
	}
}

void GameScene::Update() {

	player_->Update();

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	player_->Draw();

	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			modelBlocks_[i][j]->Draw(*transformBlocks_[i][j]);
		}
	}
}