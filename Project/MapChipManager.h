#pragma once
#include "Satlib.h"
#include "MapChipField.h"
#include "./Managers/StageManager.h"
struct CollisionMapInfo {
	bool isCellingCollision = false;
	bool isLanding = false;
	bool isWallCollision = false;
	Vector3 movementAmount;
	float width = 0.8f;
	float height = 0.8f;
};

class MapChipManager{
public:
	enum Corner {
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,

		kNumCornter,
	};

	static MapChipManager* GetInstance();

	void Initialize();

	void Update();

	void Draw();

	void Finalize();

	// 2 移動量を加味して衝突判定を処理.
	void MapCollision(const Vector3& position,CollisionMapInfo& info);

	void MapCollisionUp(const Vector3& position, CollisionMapInfo& info);
	void MapCollisionDown(const Vector3& position, CollisionMapInfo& info);
	void MapCollisionRight(const Vector3& position, CollisionMapInfo& info);
	void MapCollisionLeft(const Vector3& position, CollisionMapInfo& info);

	Vector3 CornerPosition(const Vector3& position,float width,float height, Corner corner);

	void MapCollisionCenter(const Vector3& position, CollisionMapInfo& info);

	bool OnGroundCheck(const Vector3& position, const  CollisionMapInfo& info);

	MapChipField* GetMapChipField() { return mapChipField_.get(); };

	void DeleteBlock(const Vector3& position,int offsetX,int offsetY);

	void CreateBlock(const Vector3& position);
private:
	void CreateStage();

	void GenerateFieldObjects();

	void ClearFieldObjects();
private:

	//static inline const float kWidth = 0.8f;
	//static inline const float kHeight = 0.8f;
	static inline const float kBlank = 0.2f;

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

