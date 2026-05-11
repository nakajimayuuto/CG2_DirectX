#pragma once
#include "Satlib.h"
class BaseBullet {
public:
	virtual void Initialize(const std::string& modelName, const Vector3& position, const Vector3& velocity);

	virtual void Update();

	virtual void Draw();

	bool GetIsActive() { return isActive_; };
protected:
	bool isActive_;
	
	Transform transform_;

	Renderer::ModelBox model_;
};