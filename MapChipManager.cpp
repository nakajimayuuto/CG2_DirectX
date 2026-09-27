#include "MapChipManager.h"

void MapChipManager::Initialize() {
	CreateStage();

	GenerateFieldObjects();

}

void MapChipManager::Update() {

}

void MapChipManager::Draw() {
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (!modelBlocks_[i][j].first) {
				continue;
			}

			modelBlocks_[i][j].first->Draw(*modelBlocks_[i][j].second);
		}
	}
}

void MapChipManager::Finalize() {
	for (std::vector<std::pair<std::unique_ptr<Model>, std::unique_ptr<Transform>>>& modelBlockLine : modelBlocks_) {
		for (std::pair<std::unique_ptr<Model>, std::unique_ptr<Transform>>& modelBlock : modelBlockLine) {
			modelBlock.first.release();
			modelBlock.second.release();
		}
	}
	modelBlocks_.clear();
}

void MapChipManager::CreateStage() {

	const StageData& stageData = StageManager::GetInstance()->GetCurrentStageData();

	std::string stageFileName = "Resource/map/" + stageData.name + ".csv";

	mapChipField_ = std::make_unique<MapChipField>();

	mapChipField_->LoadMapChipCsv(stageFileName);
}

void MapChipManager::GenerateFieldObjects() {
	// 要素数を変更する.
	kNumBlockVertical = mapChipField_->GetNumBlockVirtical();
	kNumBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	modelBlocks_.resize(kNumBlockVertical);
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		// 1列の要素数を設定(横方向のブロック数).
		modelBlocks_[i].resize(kNumBlockHorizontal);

	}

	//BaseEnemy* newEnemy;

	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			switch (mapChipField_->GetMapChipTypeByIndex(j, i)) {
			case MapChipType::kBlock:
				modelBlocks_[i][j].first = std::make_unique<Model>();
				modelBlocks_[i][j].first->Initialize("block");

				modelBlocks_[i][j].second = std::make_unique<Transform>();
				modelBlocks_[i][j].second->Initialize();
				modelBlocks_[i][j].second->translate = mapChipField_->GetMapChipPositionByIndex(j, i);
				break;
			case MapChipType::kPlayer:
				//assert(player_ == nullptr && "自キャラを二重に配置しようとしています");
				//player_ = new Player();
				//player_->Initialize(mapChipField_->GetMapChipPositionByIndex(j, i));
				//player_->SetMapChipField(mapChipField_);
				break;
			case MapChipType::kEnemy:
				//uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				//switch (subID) {
				//case 0:
				//	newEnemy = new Enemy();
				//	newEnemy->Initialize(mapChipField_->GetMapChipPositionByIndex(j, i));
				//
				//	enemies_.push_back(newEnemy);
				//	break;
				//case 1:
				//	newEnemy = new ShieldEnemy();
				//	newEnemy->Initialize(mapChipField_->GetMapChipPositionByIndex(j, i));
				//
				//	enemies_.push_back(newEnemy);
				//	break;
				//}
				break;
			}
		}
	}
}