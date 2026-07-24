#pragma once
#include "../Satlib.h"
#include "IScene.h"
#include "../Skydome.h"

// リストボックスに入れるアイテムの構造体.
struct GuiItem {
	const char* name;
	bool isSelect;
};

// 三角形を複数生成しやすくするための構造体. 
struct DrawModelData {
	uint32_t number;
	Model model;
	ModelSphere sphere;

	Transform transform;

	GuiItem lightingType[3];

	GuiItem textureType[3];

	bool isDelete;
};

class TitleScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void CreateModel(const Vector3& position);
private:
	std::vector<std::unique_ptr<DrawModelData>> modelDatas_;

	uint32_t modelIndex_ = 0;
	std::unique_ptr<Skydome> skydome_;
};