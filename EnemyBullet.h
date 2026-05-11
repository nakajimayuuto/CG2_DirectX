#pragma once
#include "BaseBullet.h"
#include "Satlib.h"

class EnemyBullet : public BaseBullet{
public:
	void Initialize(const std::string& modelName, const Vector3& position, const Vector3& velocity) override;

	void Update() override;

	void Draw() override;

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

