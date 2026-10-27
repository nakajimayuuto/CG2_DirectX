#include "MapChipManager.h"
#include "EnemyManager.h"

MapChipManager* MapChipManager::GetInstance() {
	static MapChipManager instance;
	return &instance;
}

void MapChipManager::Initialize() {
	StageManager::GetInstance()->LoadStageDataFile();

	mapChipField_ = std::make_unique<MapChipField>();
	mapChipField_->ResetMapChipData();
	ClearFieldObjects();

	CreateStage();

	GenerateFieldObjects();

	isWireFlame_ = false;
#ifdef _DEBUG
	isWireFlame_ = true;
#endif // _DEBUG

}

void MapChipManager::Update() {

}

void MapChipManager::Draw() {
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (!modelBlocks_[i][j].first) {
				continue;
			}

			if (isWireFlame_) {
				MapChipField::IndexSet indexSet;
				indexSet = mapChipField_->GetMapChipIndexSetByPosition(modelBlocks_[i][j].second->translate);
				uint32_t subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
				Vector4 color = { 0.2f,0.1f,0.1f,1.0f };
				if (subID == 2) {
					color = { 0.01f,0.01f,0.01f,1.0f };
				}
				Renderer::GetInstance()->DrawBoxWireFrame(*modelBlocks_[i][j].second, { 1.0f,1.0f,1.0f }, color);
			} else {
				modelBlocks_[i][j].first->Draw(*modelBlocks_[i][j].second);
			}
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

uint32_t MapChipManager::DeleteBlock(const Vector3& position, int offsetX, int offsetY) {
	MapChipType mapChipType;
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(position);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + offsetX, mapChipField_->GetNumBlockVertical() - 1 - offsetY - indexSet.yIndex);
	uint32_t subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex + offsetX, mapChipField_->GetNumBlockVertical() - 1 - offsetY - indexSet.yIndex);

	if (mapChipType != MapChipType::kBlock) {
		return -1;
	}

	if (subID == 2) {
		return-1;
	}

	uint32_t returnId = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex + offsetX, mapChipField_->GetNumBlockVertical() - 1 - offsetY - indexSet.yIndex);
	mapChipField_->SetMapChipTypeByIndexType(indexSet.xIndex + offsetX, mapChipField_->GetNumBlockVertical() - 1 - offsetY - indexSet.yIndex, MapChipType::kBlank);

	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		if (i != mapChipField_->GetNumBlockVertical() - 1 - offsetY - indexSet.yIndex) {
			continue;
		}
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (j != indexSet.xIndex + offsetX) {
				continue;
			}
			modelBlocks_[i][j].first.release();

			break;
		}
	}

	return returnId;
}

uint32_t MapChipManager::SuperDeleteBlock(const Vector3& position){
	MapChipType mapChipType;
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(position);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 -indexSet.yIndex);
	uint32_t subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 -  indexSet.yIndex);

	if (mapChipType != MapChipType::kBlock) {
		return -1;
	}

	uint32_t returnId = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex , mapChipField_->GetNumBlockVertical() - 1  - indexSet.yIndex);
	mapChipField_->SetMapChipTypeByIndexType(indexSet.xIndex , mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex, MapChipType::kBlank);

	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		if (i != mapChipField_->GetNumBlockVertical() - 1  - indexSet.yIndex) {
			continue;
		}
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (j != indexSet.xIndex) {
				continue;
			}
			modelBlocks_[i][j].first.release();

			break;
		}
	}

	return returnId;
}

void MapChipManager::CreateBlock(const Vector3& position) {
	CreateBlock(position, 1);
}

void MapChipManager::CreateBlock(const Vector3& position, uint32_t subID) {
	MapChipType mapChipType;
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(position);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		return;
	}

	mapChipField_->SetMapChipTypeByIndexType(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex, MapChipType::kBlock);
	mapChipField_->SetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex, subID);

	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		if (i != mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex) {
			continue;
		}
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (j != indexSet.xIndex) {
				continue;
			}
			modelBlocks_[i][j].first = std::make_unique<Model>();
			modelBlocks_[i][j].first->Initialize("block_template");

			switch (subID) {
			case 2:
				modelBlocks_[i][j].first->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("heavy_block"));
				break;
			case 3:
				modelBlocks_[i][j].first->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("soul_block"));
				break;
			case 4:
				modelBlocks_[i][j].first->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("explode_block"));
				break;
			}

			modelBlocks_[i][j].second = std::make_unique<Transform>();
			modelBlocks_[i][j].second->Initialize();
			modelBlocks_[i][j].second->translate = mapChipField_->GetMapChipPositionByIndex(j, i);

			break;
		}
	}
}

void MapChipManager::ChangeSolidMode() {
	if (isWireFlame_) {
		isWireFlame_ = false;
	} else {
		isWireFlame_ = true;
	}
}

void MapChipManager::ResetBlock(uint32_t leftTopIndexX, uint32_t leftTopIndexY, uint32_t rightBottomIndexX, uint32_t rightBottomIndexY, bool isPhase1) {
	uint8_t subID = 0;
	std::string modelName;
	for (uint32_t i = leftTopIndexY; i < rightBottomIndexY; ++i) {
		for (uint32_t j = leftTopIndexX; j < rightBottomIndexX; ++j) {
			//modelBlocks_[i][j].first = nullptr;
			if (isPhase1) {
				if (i >= 13 && i <= 16) {
					subID = 1;
					if (modelBlocks_[i][j].first) {
						if (subID == mapChipField_->GetMapChipSubIDByIndex(j, i)) {
							continue;
						}
					}
				} else if (i == 17) {
					subID = 2;
					if (modelBlocks_[i][j].first) {
						if (subID == mapChipField_->GetMapChipSubIDByIndex(j, i)) {
							continue;
						}
					}
				} else {
					if (!modelBlocks_[i][j].first) {
						continue;
					}
				}
			} else {
				if (i >= 13 && i <= 15) {
					if (j >= 107 && j <= 127) {
						subID = 1;
						if (modelBlocks_[i][j].first) {
							if (subID == mapChipField_->GetMapChipSubIDByIndex(j, i)) {
								continue;
							}
						}
					} else {
						if (!modelBlocks_[i][j].first) {
							continue;
						}
					}
				} else {
					if (!modelBlocks_[i][j].first) {
						continue;
					}
				}
			}

			switch (subID) {
			case 0:
				modelBlocks_[i][j].first.reset();
				mapChipField_->SetMapChipTypeByIndexType(j, i, MapChipType::kBlank);
				for (uint32_t k = 0; k < 10; k++) {
					ParticleManager::GetInstance()->SpawnParticles("death_cross", mapChipField_->GetMapChipPositionByIndex(j, i));
				}
				break;
			case 1:
				modelBlocks_[i][j].first = std::make_unique<Model>();
				modelBlocks_[i][j].first->Initialize("block_template");

				mapChipField_->SetMapChipTypeByIndexType(j, i, MapChipType::kBlock);
				mapChipField_->SetMapChipSubIDByIndex(j, i, 1);

				modelBlocks_[i][j].second = std::make_unique<Transform>();
				modelBlocks_[i][j].second->Initialize();
				modelBlocks_[i][j].second->translate = mapChipField_->GetMapChipPositionByIndex(j, i);
				for (uint32_t k = 0; k < 10; k++) {
					ParticleManager::GetInstance()->SpawnParticles("death_cross", mapChipField_->GetMapChipPositionByIndex(j, i));
				}
				break;
			case 2:
				modelBlocks_[i][j].first = std::make_unique<Model>();
				modelBlocks_[i][j].first->Initialize("block_template");
				modelBlocks_[i][j].first->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("heavy_block"));

				mapChipField_->SetMapChipTypeByIndexType(j, i, MapChipType::kBlock);
				mapChipField_->SetMapChipSubIDByIndex(j, i, 2);

				modelBlocks_[i][j].second = std::make_unique<Transform>();
				modelBlocks_[i][j].second->Initialize();
				modelBlocks_[i][j].second->translate = mapChipField_->GetMapChipPositionByIndex(j, i);
				for (uint32_t k = 0; k < 10; k++) {
					ParticleManager::GetInstance()->SpawnParticles("death_cross", mapChipField_->GetMapChipPositionByIndex(j, i));
				}
				break;
			}

			continue;
			subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
			switch (mapChipField_->GetMapChipTypeByIndex(j, i)) {
			case MapChipType::kBlock:
				modelBlocks_[i][j].first = std::make_unique<Model>();
				modelBlocks_[i][j].first->Initialize("block_template");

				switch (subID) {
				case 2:
					modelBlocks_[i][j].first->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("heavy_block"));
					break;
				case 3:
					modelBlocks_[i][j].first->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("soul_block"));
					break;
				case 4:
					modelBlocks_[i][j].first->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("explode_block"));
					break;
				}

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
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				switch (subID) {
				case 0:
					EnemyManager::GetInstance()->CreateEnemy(mapChipField_->GetMapChipPositionByIndex(j, i), EnemyManager::kDummy);
					break;
				case 1:
					break;
				}
				break;
			}
		}
	}
}

void MapChipManager::CreateStage() {

	const StageData& stageData = StageManager::GetInstance()->GetCurrentStageData();

	std::string stageFileName = "Resource/map/" + stageData.name + ".csv";

	mapChipField_->LoadMapChipCsv(stageFileName);
}

void MapChipManager::GenerateFieldObjects() {
	// 要素数を変更する.
	kNumBlockVertical = mapChipField_->GetNumBlockVertical();
	kNumBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	modelBlocks_.resize(kNumBlockVertical);
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		// 1列の要素数を設定(横方向のブロック数).
		modelBlocks_[i].resize(kNumBlockHorizontal);

	}

	//BaseEnemy* newEnemy;
	uint8_t subID;
	std::string modelName;
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
			switch (mapChipField_->GetMapChipTypeByIndex(j, i)) {
			case MapChipType::kBlock:
				modelBlocks_[i][j].first = std::make_unique<Model>();
				modelBlocks_[i][j].first->Initialize("block_template");

				switch (subID) {
				case 2:
					modelBlocks_[i][j].first->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("heavy_block"));
					break;
				case 3:
					modelBlocks_[i][j].first->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("soul_block"));
					break;
				case 4:
					modelBlocks_[i][j].first->ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("explode_block"));
					break;
				}

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
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				switch (subID) {
				case 0:
					EnemyManager::GetInstance()->CreateEnemy(mapChipField_->GetMapChipPositionByIndex(j, i), EnemyManager::kDummy);
					break;
				case 1:
					break;
				}
				break;
			}
		}
	}
}

void MapChipManager::ClearFieldObjects() {
	if (modelBlocks_.size() <= 0) {
		return;
	}

	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			modelBlocks_[i][j].first.release();
			modelBlocks_[i][j].second.release();
		}
	}
}

void MapChipManager::MapCollision(const Vector3& position, CollisionMapInfo& info) {
	MapCollisionUp(position, info);
	MapCollisionDown(position, info);
	MapCollisionRight(position, info);
	MapCollisionLeft(position, info);
	MapCollisionCenter(position, info);
}

void MapChipManager::MapCollisionNoCenter(const Vector3& position, CollisionMapInfo& info) {
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
		positionNew[i] = CornerPosition(static_cast<Vector3>(position) + info.movementAmount, info.width, info.height, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
		if (mapChipType != MapChipType::kBlank) {
			info.type = mapChipType;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
		}
		if (mapChipTypeNext != MapChipType::kBlank) {
			info.type = mapChipTypeNext;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - indexSet.yIndex);
		}
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
		if (mapChipType != MapChipType::kBlank) {
			info.type = mapChipType;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
		}
		if (mapChipTypeNext != MapChipType::kBlank) {
			info.type = mapChipTypeNext;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - indexSet.yIndex);
		}
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(static_cast<Vector3>(position), info.width, info.height, kLeftTop));

		if (indexSetNow.yIndex != indexSet.yIndex) {

			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.y = std::max(0.0f, (rect.bottom - static_cast<Vector3>(position).y) - ((info.height / 2.0f) + kBlank));
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
		positionNew[i] = CornerPosition(static_cast<Vector3>(position) + info.movementAmount, info.width, info.height, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 2 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
		if (mapChipType != MapChipType::kBlank) {
			info.type = mapChipType;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
		}
		if (mapChipTypeNext != MapChipType::kBlank) {
			info.type = mapChipTypeNext;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - indexSet.yIndex);
		}
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 2 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
		if (mapChipType != MapChipType::kBlank) {
			info.type = mapChipType;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
		}
		if (mapChipTypeNext != MapChipType::kBlank) {
			info.type = mapChipTypeNext;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - indexSet.yIndex);
		}
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(static_cast<Vector3>(position), info.width, info.height, kLeftBottom));

		if (indexSetNow.yIndex != indexSet.yIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.y = std::min(0.0f, (rect.top - static_cast<Vector3>(position).y) + ((info.height / 2.0f) + kBlank));
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
		positionNew[i] = CornerPosition(static_cast<Vector3>(position) + info.movementAmount, info.width, info.height, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	//MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
		if (mapChipType != MapChipType::kBlank) {
			info.type = mapChipType;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
		}
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
		if (mapChipType != MapChipType::kBlank) {
			info.type = mapChipType;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
		}
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(static_cast<Vector3>(position), info.width, info.height, kRightTop));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.x = std::max(0.0f, (rect.left - static_cast<Vector3>(position).x) - ((info.width / 2.0f) + kBlank));
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
		positionNew[i] = CornerPosition(static_cast<Vector3>(position) + info.movementAmount, info.width, info.height, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	//MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock	

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
		if (mapChipType != MapChipType::kBlank) {
			info.type = mapChipType;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
		}
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
		if (mapChipType != MapChipType::kBlank) {
			info.type = mapChipType;
			info.subID = mapChipField_->GetMapChipSubIDByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);
		}
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(static_cast<Vector3>(position), info.width, info.height, kLeftTop));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.x = std::min(0.0f, (rect.right - static_cast<Vector3>(position).x) + ((info.width / 2.0f) + kBlank));
			info.isWallCollision = true;
		}
	}
}

Vector3 MapChipManager::CornerPosition(const Vector3& position, float width, float height, Corner corner) {
	Vector3 offsetTable[kNumCornter] = {
		{+width / 2.0f, -height / 2.0f, 0.0f},
		{-width / 2.0f, -height / 2.0f, 0.0f},
		{+width / 2.0f, +height / 2.0f, 0.0f},
		{-width / 2.0f, +height / 2.0f, 0.0f}
	};

	return static_cast<Vector3>(position) + offsetTable[static_cast<uint32_t>(corner)];
}

void MapChipManager::MapCollisionCenter(const Vector3& position, CollisionMapInfo& info) {
	float newPosY = 0.1f;
	MapChipType mapChipType;

	MapChipField::IndexSet indexSet;
	MapChipField::IndexSet preIndexSet;
	preIndexSet = mapChipField_->GetMapChipIndexSetByPosition(static_cast<Vector3>(position));
	while (true) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(static_cast<Vector3>(position) + Vector3(0.0f, newPosY, 0.0f));
		mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);

		if (mapChipType == MapChipType::kBlock) {
			newPosY += 1.0f;

			if (indexSet.xIndex < 0 || static_cast<int32_t>(kNumBlockHorizontal) - 1 < indexSet.xIndex ) {
				break;
			}

			if (indexSet.yIndex < 0 || static_cast<int32_t>(kNumBlockVertical) - 1 < indexSet.yIndex) {
				break;
			}
		} else {
			break;
		}
	}

	if (newPosY > 0.1f) {
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(static_cast<Vector3>(position), info.width, info.height, kLeftBottom));

		if (indexSetNow.yIndex != indexSet.yIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.y = std::min(0.0f, (rect.top - static_cast<Vector3>(position).y) + ((info.height / 2.0f) + kBlank));
			info.movementAmount.y += newPosY;
		}
	}
	//indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	//MapChipField::IndexSet indexSetNow;
	//indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(static_cast<Vector3>(position), info.width, info.height, kLeftTop));
	//
	//if (indexSetNow.xIndex != indexSet.xIndex) {
	//	MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
	//	info.movementAmount.x = std::min(0.0f, (rect.right - static_cast<Vector3>(position).x) + ((info.width / 2.0f) + kBlank));
	//	info.isWallCollision = true;
	//}
	//
}

bool MapChipManager::OnGroundCheck(const Vector3& position, const CollisionMapInfo& info) {
	std::array<Vector3, 4> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); i++) {
		positionNew[i] = MapChipManager::GetInstance()->CornerPosition(static_cast<Vector3>(position) + info.movementAmount, info.width, info.height, static_cast<MapChipManager::Corner>(i));
	}

	MapChipType mapChipType;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom] + Vector3(0.0f, -kBlank, 0.0f));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom] + Vector3(0.0f, -kBlank, 0.0f));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVertical() - 1 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	//if (!hit) {
	//	return false;
	//}

	return hit;
}
