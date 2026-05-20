#pragma once
#include "Satlib.h"

class RailCameraController{
public:
	enum class CameraType {
		kFirstPoint,
		kThirdPoint,
	};
	~RailCameraController();

	void Initialize(const Transform& transform);
	void Update();
	void Draw();

	void SetTargetMatrix(Matrix4x4 matrix) { matrix_ = matrix; };

	CameraType GetCameraType() { return type_; };

	const Transform& GetTransform() { return transform_; };
private:
	Transform transform_;

	Camera* camera_;

	std::vector<Vector3> controlPoints_;

	float timer_;

	Matrix4x4 matrix_;

	CameraType type_;

	const float timeMax = 150.0f;
};

