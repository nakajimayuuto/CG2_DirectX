#pragma once
#include "Satlib.h"

class Player;

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

	void SetTarget(Player* player) { player_ = player; };

	CameraType GetCameraType() { return type_; };

	const Transform& GetTransform() { return transform_; };
private:
	Transform transform_;

	Camera* camera_;

	std::vector<Vector3> controlPoints_;

	float timer_;

	Player* player_ = nullptr;

	CameraType type_;

	const float timeMax = 150.0f;
};

