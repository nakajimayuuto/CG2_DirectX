#pragma once
#include "Satlib.h"
enum class BulletType {
	kNormal,
	kBounce,
	kFire,
	kSpike,
};

class Bullet :Collider{
public:
	void Initialize(const Transform& transform,const Vector3& velocity, BulletType type);

	void Update();

	void Draw();

	bool GetIsActive() { return isActive_; };
private:
	static void (Bullet::* pInitializeFunc[])();
	static void (Bullet::* pUpdateFunc[])();

	void NormalInitialize();
	void NormalUpdate();

private:
	Transform modelTransform_;
	Model model_;
	Vector3 velocity_;
	BulletType type_;

	float lifeTimer_ = 0.0f;
	static inline float lifeTimeMax_ = 3.0f;

	bool isActive_;
};

class Explode : Collider {

};

class ProjectileManager {
public:
	static ProjectileManager* GetInstance();

	void Initialize();

	void Update();

	void Draw();

	void CreateBullet(const Transform& transform, const Vector3& velocity, BulletType type);
private:
	std::vector<std::unique_ptr<Bullet>> bullets;
};