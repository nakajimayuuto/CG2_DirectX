#pragma once
#include "../Satlib.h"
#include "IScene.h"

struct DrawModelData {
	uint32_t number_;
	Model model;
	ModelSphere sphere;
};

class TitleScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void CreateModel(const Vector3& position);
private:
	std::vector<std::unique_ptr<DrawModelData>> modelDatas_;
};