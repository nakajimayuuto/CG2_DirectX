#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
class ForwardEnemy : public BaseEnemy{
public:
	void Initialize(Vector3 position) override;
	void Update() override;
	void Draw() override;

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	static inline float kSpeed = 0.1f;
	Vector3 velocity_;

	Transform transform_;
	Renderer::ModelBox model_;
};

