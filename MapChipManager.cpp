#include "MapChipManager.h"

MapChipManager* MapChipManager::GetInstance() {
	static MapChipManager instance;
	return &instance;
}

void MapChipManager::Initialize() {
	StageManager::GetInstance()->LoadStageDataFile();

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
				modelBlocks_[i][j].first->Initialize("block_template");

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

void MapChipManager::MapCollision(const Vector3& position, CollisionMapInfo& info) {
	MapCollisionUp(position, info);
	MapCollisionDown(position, info);
	MapCollisionRight(position, info);
	MapCollisionLeft(position, info);
}

void MapChipManager::MapCollisionUp(const Vector3& position, CollisionMapInfo& info) {
	if (info.movementAmount.y <= 0.0f) {
		return;
	}

	std::array<Vector3, 4> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); i++) {
		positionNew[i] = CornerPosition(static_cast<Vector3>(position) + info.movementAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(static_cast<Vector3>(position), kLeftTop));

		if (indexSetNow.yIndex != indexSet.yIndex) {

			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.y = std::max(0.0f, (rect.bottom - static_cast<Vector3>(position).y) - ((kHeight / 2.0f) + kBlank));
			info.isCellingCollision = true;
		}
	}
}

void MapChipManager::MapCollisionDown(const Vector3& position, CollisionMapInfo& info) {
	if (info.movementAmount.y >= 0.0f) {
		return;
	}

	std::array<Vector3, 4> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); i++) {
		positionNew[i] = CornerPosition(static_cast<Vector3>(position) + info.movementAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 2 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 2 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(static_cast<Vector3>(position), kLeftBottom));

		if (indexSetNow.yIndex != indexSet.yIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.y = std::min(0.0f, (rect.top - static_cast<Vector3>(position).y) + ((kHeight / 2.0f) + kBlank));
			info.isLanding = true;
		}
	}
}

void MapChipManager::MapCollisionRight(const Vector3& position, CollisionMapInfo& info) {
	if (info.movementAmount.x <= 0.0f) {
		return;
	}

	std::array<Vector3, 4> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); i++) {
		positionNew[i] = CornerPosition(static_cast<Vector3>(position) + info.movementAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	//MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(static_cast<Vector3>(position), kRightTop));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.x = std::max(0.0f, (rect.left - static_cast<Vector3>(position).x) - ((kWidth / 2.0f) + kBlank));
			info.isWallCollision = true;
		}
	}
}

void MapChipManager::MapCollisionLeft(const Vector3& position, CollisionMapInfo& info) {
	if (info.movementAmount.x >= 0.0f) {
		return;
	}

	std::array<Vector3, 4> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); i++) {
		positionNew[i] = CornerPosition(static_cast<Vector3>(position) + info.movementAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	//MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock	

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(static_cast<Vector3>(position), kLeftTop));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.x = std::min(0.0f, (rect.right - static_cast<Vector3>(position).x) + ((kWidth / 2.0f) + kBlank));
			info.isWallCollision = true;
		}
	}
}

Vector3 MapChipManager::CornerPosition(const Vector3& position, Corner corner) {
	Vector3 offsetTable[kNumCornter] = {
		{+kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
		{-kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
		{+kWidth / 2.0f, +kHeight / 2.0f, 0.0f},
		{-kWidth / 2.0f, +kHeight / 2.0f, 0.0f}
	};

	return static_cast<Vector3>(position) + offsetTable[static_cast<uint32_t>(corner)];
}

bool MapChipManager::OnGroundCheck(const Vector3& position,const CollisionMapInfo& info){
	std::array<Vector3, 4> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); i++) {
		positionNew[i] = MapChipManager::GetInstance()->CornerPosition(static_cast<Vector3>(position) + info.movementAmount, static_cast<MapChipManager::Corner>(i));
	}

	MapChipType mapChipType;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom] + Vector3(0.0f, -kBlank, 0.0f));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom] + Vector3(0.0f, -kBlank, 0.0f));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	//if (!hit) {
	//	return false;
	//}

	return hit;
}
