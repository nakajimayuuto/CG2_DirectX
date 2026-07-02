#pragma once
#include "Satlib.h"
class PlaneProjectionShadow{
public:
	void Initialize(Transform* casterWorldTransform,Model* model);

	void Update();

	void Draw();
private:
	Transform transform_;
	
	Model* model_ = nullptr;

	Transform* casterTransform_ = nullptr;

	Matrix4x4 shadowMatrix_;
};

