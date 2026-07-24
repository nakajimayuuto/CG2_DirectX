#pragma once
#include "../Satlib.h"
#include "IScene.h"
#include "../Skydome.h"

// リストボックスに入れるアイテムの構造体.
struct GuiItem {
	const char* name;
	bool isSelect;
};

enum class DrawModelType {
	Plane,
	Sphere,
	UtahTeapot,
	StanfordBunny,
	MultiMesh,
	MultiMaterial,
	Suzzanne,
};

// 三角形を複数生成しやすくするための構造体. 
struct DrawModelData {
	uint32_t number;
	Model model;
	TextureInfo sphereInfo;

	Transform transform;

	GuiItem lightingType[3];

	GuiItem textureType[3];

	DrawModelType type;

	bool isDelete;

	bool isMultiMesh;
};

class TitleScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void CreateModelData(const Vector3& position, DrawModelType type);

	void CreateModel(DrawModelData* data);
private:
	std::vector<std::unique_ptr<DrawModelData>> modelDatas_;

	uint32_t modelIndex_ = 0;

	DrawModelType currentNewModelType_;
};