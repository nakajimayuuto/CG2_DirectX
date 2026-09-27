#pragma once
#include "Satlib.h"
#include "MapChipField.h"
#include "./Managers/StageManager.h"
class MapChipManager{
public:
	void Initialize();

	void Update();

	void Draw();

	void Finalize();
private:
	void CreateStage();

	void GenerateFieldObjects();
private:

	// 要素数.
	uint32_t kNumBlockVertical = 20;
	uint32_t kNumBlockHorizontal = 10;
	// ブロック1個分の横幅.
	const float kBlockWidth = 1.0f;
	const float kBlockHeight = 1.0f;
	std::unique_ptr<MapChipField> mapChipField_ = nullptr;

	//std::vector<std::vector<>> transformBlocks_;
	std::vector<std::vector<std::pair<std::unique_ptr<Model>,std::unique_ptr<Transform>>>> modelBlocks_;
};

