#pragma once
#include "Satlib.h"
class RailCameraController{
public:
	void Initialize(const Transform& transform);
	void Update();
	void Draw();

	const Transform& GetTransform() { return transform_; };
private:
	Transform transform_;

	Camera* camera_;

	std::vector<Vector3> controlPoints_;

	float timer_;

	const float timeMax = 150.0f;
};

