#pragma once
#include "Satlib.h"
#include "BaseBullet.h"

class PlayerBullet : public BaseBullet{
public:
	void Initialize(const std::string& modelName,const Vector3& position, const Vector3& velocity) override;

	void Update() override;

	void Draw() override;

	void OnCollision() override;

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	void LifeTimeUpdate();
private:
	Vector3 velocity_;

	// タイマー
	static inline float kLifeTime = 5.0f;
	float deathTimer_;
};

