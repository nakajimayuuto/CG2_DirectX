#pragma once
#include "Satlib.h"
#include <string>
#include <vector>

enum class MapChipType {
	kBlank, // 空白.
	kBlock, // ブロック.
	kPlayer, // プレイヤー.
	kEnemy, // 敵.
};

struct MapChipDataUnit {
	MapChipType type; // マップチップの種別.
	uint8_t subID; // 種類ごとのサブID.
};

struct MapChipData {
	std::vector<std::vector<MapChipDataUnit>> data;
};

class MapChipField {
public:
	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	struct Rect {
		float left;
		float right;
		float bottom;
		float top;
	};

	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	uint8_t GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex);

	uint32_t GetNumBlockVirtical() { return kNumBlockVertical; };
	uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; };

	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);
private:
	enum MapChipCharIndex {
		kChipType = 0, // マップチップタイプ.
		kChipSubID = 1, // タイプごとのサブID.
	};

	// 幅
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// 要素数
	static inline const uint32_t kNumBlockVertical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	MapChipData mapChipData_;

};