#include "ProjectileManager.h"
ProjectileManager* ProjectileManager::GetInstance() {
	static ProjectileManager instance;
	return &instance;
}

void ProjectileManager::Initialize() {
	bullets.clear();

}

void ProjectileManager::Update() {
	for (auto& bullet : bullets) {
		bullet->Update();
	}

	std::erase_if(bullets, [](const std::unique_ptr<Bullet>& bullet) {
		return !bullet->GetIsActive(); }
	);
}

void ProjectileManager::Draw() {
	for (auto& bullet : bullets) {
		bullet->Draw();
	}

}

void ProjectileManager::CreateBullet(const Transform& transform, const Vector3& velocity, BulletType type) {
	std::unique_ptr<Bullet> bullet;
	bullet = std::make_unique<Bullet>();
	bullet->Initialize(transform, velocity, type);
	bullets.push_back(std::move(bullet));
}

void (Bullet::* Bullet::pInitializeFunc[])() = {
		&Bullet::NormalInitialize,
};

void (Bullet::* Bullet::pUpdateFunc[])() = {
		&Bullet::NormalUpdate,
};

void Bullet::Initialize(const Transform& transform, const Vector3& velocity, BulletType type) {
	transform_ = transform;
	modelTransform_.Initialize();
	modelTransform_.SetParent(&transform_);
	velocity_ = velocity;
	type_ = type;
	lifeTimer_ = 0.0f;
	lifeTimeMax_ = 60.0f;
	isActive_ = true;

	(this->*pInitializeFunc[static_cast<size_t>(type_)])();
}

void Bullet::Update() {
	lifeTimer_ += DeltaTime::GetInstance()->GetDeltaTime();
	(this->*pUpdateFunc[static_cast<size_t>(type_)])();

	if (lifeTimer_ >= lifeTimeMax_) {
		isActive_ = false;
	}
}

void Bullet::Draw() {
	Renderer::GetInstance()->DrawSphereWireFrame(modelTransform_, { 1.0f,0.0f,0.0f,1.0f });
}

void Bullet::NormalInitialize() {
	transform_.rotate.y = std::atan2(velocity_.x, velocity_.z);
}

void Bullet::NormalUpdate() {
	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetDeltaTime();
}
