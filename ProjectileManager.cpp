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

	for (auto it = bullets.begin(); it != bullets.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = bullets.erase(it);
		} else {
			++it;
		}
	}
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

void ProjectileManager::CreateDiffusionBullet(const Transform& transform, const Vector3& velocity, BulletType type, float diffusionRadian, uint32_t amount) {
	float centerRadian = std::atan2(velocity.x, velocity.z);
	float speedY = velocity.y * Vector3(0.0f,velocity.y,0.0f).Length();
	float xzLenght = Vector3(velocity.x, 0.0f, velocity.z).Length();
	Vector3 newVelocity = {0.0f,0.0f,1.0f};
	float newRadian = 0.0f;
	float startRadian = 0.0f;

	if (amount % 2 == 0) {
		startRadian = centerRadian - (diffusionRadian * (amount / 2)) + (diffusionRadian / 2.0f) + Radian(90.0f);
		for (uint32_t i = 0; i < amount; i++) {
			newRadian = startRadian + (diffusionRadian * i);
			newVelocity.x = -RadianToVector(newRadian).x * xzLenght;
			newVelocity.y = speedY;
			newVelocity.z = RadianToVector(newRadian).y * xzLenght;
			CreateBullet(transform,newVelocity,type);
		}
	} else {
		startRadian = centerRadian - (diffusionRadian * ((amount - 1) / 2)) + Radian(90.0f);
		for (uint32_t i = 0; i < amount; i++) {
			newRadian = startRadian + (diffusionRadian * i);
			newVelocity.x = -RadianToVector(newRadian).x * xzLenght;
			newVelocity.y = speedY;
			newVelocity.z = RadianToVector(newRadian).y * xzLenght;
			CreateBullet(transform, newVelocity, type);
		}
	}
}

void (Bullet::* Bullet::pInitializeFunc[])() = {
		&Bullet::NormalInitialize,
		&Bullet::BounsInitialize,
};

void (Bullet::* Bullet::pUpdateFunc[])() = {
		&Bullet::NormalUpdate,
		&Bullet::BounsUpdate,
};

void Bullet::Initialize(const Transform& transform, const Vector3& velocity, BulletType type) {
	transform_.scale = transform.scale;
	transform_.rotate = transform.rotate;
	transform_.translate = transform.GetWorldPosition();
	modelTransform_.Initialize();
	modelTransform_.SetParent(&transform_);
	velocity_ = velocity;
	type_ = type;
	lifeTimer_ = 0.0f;
	lifeTimeMax_ = kBasicLifeTimeMax_;
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

void Bullet::BounsInitialize() {
	velocity_.y = 1.0f;

	lifeTimeMax_ = 5.0f;
}

void Bullet::BounsUpdate() {
	velocity_.y -= gravityAcceleration_ * DeltaTime::GetInstance()->GetDeltaTime();
	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetDeltaTime();

	if (transform_.GetWorldPosition().y - (radius_ / 2.0f) <= 0.0f) {
		transform_.translate.y = (radius_ / 2.0f);

		Vector3 reflected = velocity_.Reflect({ 0.0f,1.0f,0.0f });
		Vector3 projectToNormal = Vector3::Project(reflected, { 0.0f,1.0f,0.0f });
		Vector3 movingDirection = reflected - projectToNormal;
		velocity_ = projectToNormal * kBounsE_ + movingDirection;
	}
}
