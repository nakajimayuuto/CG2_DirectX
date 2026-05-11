#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
class ForwardEnemy : public BaseEnemy{
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	static inline float kSpeed = 1.0f;
	Vector3 velocity_;

	Transform transform_;
	Renderer::ModelBox model_;
};

