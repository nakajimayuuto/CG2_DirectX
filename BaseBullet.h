#pragma once
#include "Satlib.h"
#include "Collider.h"

class BaseBullet : public Collider{
public:
	virtual void Initialize(const std::string& modelName, const Vector3& position, const Vector3& velocity) = 0;

	virtual void Update() = 0;

	virtual void Draw() = 0;

	void OnCollision()override = 0;

	bool GetIsActive() { return isActive_; };
	Vector3 GetWorldPosition() override { return { transform_.GetAffineMatrix().matrix[3][0],transform_.GetAffineMatrix().matrix[3][1],transform_.GetAffineMatrix().matrix[3][2] }; };
	Transform GetTransform()const { return transform_; };
protected:
	bool isActive_;
	
	Transform transform_;

	Renderer::ModelBox model_;
};