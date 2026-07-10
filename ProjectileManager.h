#pragma once
#include "Satlib.h"
class ProjectileManager{
public:
	ProjectileManager* GetInstance();

	void Initialize();

	void Update();

	void Draw();
};

class Bullet :Collider{
public:
	enum class BulletType{
		kNormal,
		kBounce,
		kFire,
		kSpike,
	};


	void Initialize(const Transform& transform,const Vector3& velocity, BulletType type);

	void Update();

	void Draw();
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
};

class Explode : Collider {

};