#pragma once
#include "Satlib.h"
#include <string>
#include <vector>

enum class MapChipType {
	kBlank, // 空白.
	kBlock, // ブロック.
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {
private:
	// 幅
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	// 要素数
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	MapChipData mapChipData_;

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

	uint32_t GetNumBlockVirtical() { return kNumBlockVirtical; };
	uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; };

	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

};