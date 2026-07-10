#include "ProjectileManager.h"
ProjectileManager* ProjectileManager::GetInstance() {
	static ProjectileManager instance;
	return &instance;
}

void ProjectileManager::Initialize() {

}

void ProjectileManager::Update() {

}

void ProjectileManager::Draw() {

}

void (Bullet::* Bullet::pInitializeFunc[])() = {
		&Bullet::NormalInitialize,
};

void (Bullet::* Bullet::pUpdateFunc[])() = {
		&Bullet::NormalUpdate,
};

void Bullet::Initialize(const Transform& transform, const Vector3& velocity, BulletType type){
	transform_ = transform;
	modelTransform_.Initialize();
	modelTransform_.SetParent(&transform_);
	velocity_ = velocity;
	type_ = type;
	(this->*pInitializeFunc[static_cast<size_t>(type_)])();
}

void Bullet::Update() {
	(this->*pUpdateFunc[static_cast<size_t>(type_)])();
}

void Bullet::Draw() {
	Renderer::GetInstance()->DrawSphereWireFrame(modelTransform_, {1.0f,0.0f,0.0f,1.0f});
}

void Bullet::NormalInitialize(){
}

void Bullet::NormalUpdate(){
}
