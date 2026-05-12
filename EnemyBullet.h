#pragma once
#include "BaseBullet.h"
#include "Satlib.h"

class Player;

class EnemyBullet : public BaseBullet{
public:
	~EnemyBullet();
	void Initialize(const std::string& modelName, const Vector3& position, const Vector3& velocity) override;

	void Update() override;

	void Draw() override;

	void OnCollision() override;

	void SetPlayer(Player* player) { player_ = player; };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	void LifeTimeUpdate();
private:
	Vector3 velocity_;

	static inline float kSpeed = 0.5f;

	static inline float kHomingRatio = 0.025f;

	// タイマー
	static inline float kLifeTime = 5.0f;
	float deathTimer_;
	Player* player_ = nullptr;
};

