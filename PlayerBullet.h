#pragma once
#include "Satlib.h"

class PlayerBullet{
public:
	void Initialize(const std::string& modelName,const Vector3& position);

	void Update();

	void Draw();
private:
	Transform transform_;

	Renderer::ModelBox model_;
};

