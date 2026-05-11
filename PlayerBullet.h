#pragma once
#include "Satlib.h"

class PlayerBullet{
public:
	void Initialize(const std::string& modelName,const Vector3& position, const Vector3& velocity);

	void Update();

	void Draw();

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	bool GetIsActive() { return isActive_; };
private:
	void LifeTimeUpdate();
private:
	Vector3 velocity_;

	// タイマー
	static inline float kLifeTime = 5.0f;
	float deathTimer_;
	bool isActive_;


	Transform transform_;

	Renderer::ModelBox model_;
};

