#pragma once
#include "Satlib.h"
class RailCameraController{
public:
	void Initialize(const Transform& transform);
	void Update();

	const Transform& GetTransform() { return transform_; };
private:
	Transform transform_;

	Camera* camera_;
};

