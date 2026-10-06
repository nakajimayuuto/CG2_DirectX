#include "ProjectileManager.h"
ProjectileManager* ProjectileManager::GetInstance() {
	static ProjectileManager instance;
	return &instance;
}

void ProjectileManager::Initialize() {
	bullets.clear();
	waves.clear();
	spikes.clear();
	explodes.clear();

	for (uint32_t i = 0; i < kBulletLightMax_; i++) {
		lightNames_.push_back(std::format("bullet_{}", i));
		LightManager::GetInstance()->CreatePointLight(lightNames_[i]);
	}
}

void ProjectileManager::Update() {
	uint32_t currentLightNum_ = 0;

	for (auto& name : lightNames_) {
		LightManager::GetInstance()->SetLightIsActive(name, false);
	}

	for (auto& bullet : bullets) {
		bullet->Update();
#ifndef _DEBUG
		if (bullet->GetType() == BulletType::kNormal) {
			if (currentLightNum_ < kBulletLightMax_) {
				LightManager::GetInstance()->SetLightPos(lightNames_[currentLightNum_], bullet->GetTransform().GetWorldPosition());
				LightManager::GetInstance()->SetLightIsActive(lightNames_[currentLightNum_], true);
				currentLightNum_++;
			}
		}
#endif // _DEBUG
	}

	for (auto it = bullets.begin(); it != bullets.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = bullets.erase(it);
		} else {
			++it;
		}
	}

	for (auto& bullet : bullets) {
		CollisionManager::GetInstance()->AddColliderList(reinterpret_cast<Collider*>(bullet.get()));
	}

	for (auto& wave : waves) {
		wave->Update();
	}

	for (auto it = waves.begin(); it != waves.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = waves.erase(it);
		} else {
			++it;
		}
	}

	for (auto& wave : waves) {
		CollisionManager::GetInstance()->AddColliderList(reinterpret_cast<Collider*>(wave.get()));
	}

	for (auto& spike : spikes) {
		spike->Update();
	}

	for (auto it = spikes.begin(); it != spikes.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = spikes.erase(it);
		} else {
			++it;
		}
	}

	for (auto& spike : spikes) {
		CollisionManager::GetInstance()->AddColliderList(reinterpret_cast<Collider*>(spike.get()));
	}

	for (auto& explode : explodes) {
		explode->Update();
	}

	for (auto it = explodes.begin(); it != explodes.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = explodes.erase(it);
		} else {
			++it;
		}
	}

	for (auto& explode : explodes) {
		CollisionManager::GetInstance()->AddColliderList(reinterpret_cast<Collider*>(explode.get()));
	}

	for (auto& tutorial : tutorials) {
		tutorial->Update();
	}

	for (auto it = tutorials.begin(); it != tutorials.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = tutorials.erase(it);
		} else {
			++it;
		}
	}

	for (auto& tutorial : tutorials) {
		CollisionManager::GetInstance()->AddColliderList(reinterpret_cast<Collider*>(tutorial.get()));
	}
}

void ProjectileManager::Draw() {
	for (auto& bullet : bullets) {
		bullet->Draw();
	}

	for (auto& wave : waves) {
		wave->Draw();
	}

	for (auto& spike : spikes) {
		spike->Draw();
	}

	for (auto& tutorial : tutorials) {
		tutorial->Draw();
	}

}

void ProjectileManager::CreateBullet(const Transform& transform, const Vector3& velocity, BulletType type, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	CreateBullet(transform, velocity, type, colliderName, damage, damageCoolTime, 0);
}

void ProjectileManager::CreateBullet(const Transform& transform, const Vector3& velocity, BulletType type, CollisionAttributeName colliderName, float damage, float damageCoolTime, uint32_t diffusionAmount) {
	std::unique_ptr<Bullet> bullet;
	bullet = std::make_unique<Bullet>();
	bullet->Initialize(transform, velocity, type, colliderName, damage, damageCoolTime);
	bullet->SetDiffusionAmount(diffusionAmount);
	if (bullet->GetType() == BulletType::kNormal) {

	}
	bullets.push_back(std::move(bullet));
}

void ProjectileManager::CreateDiffusionBullet(const Transform& transform, const Vector3& velocity, BulletType type, CollisionAttributeName colliderName, float damage, float damageCoolTime, float diffusionRadian, uint32_t amount) {
	float centerRadian = std::atan2(velocity.x, velocity.z);
	float speedY = velocity.y * Vector3(0.0f, velocity.y, 0.0f).Length();
	float xzLenght = Vector3(velocity.x, 0.0f, velocity.z).Length();
	Vector3 newVelocity = { 0.0f,0.0f,1.0f };
	float newRadian = 0.0f;
	float startRadian = 0.0f;

	if (amount % 2 == 0) {
		startRadian = centerRadian - (diffusionRadian * (amount / 2)) + (diffusionRadian / 2.0f) + Radian(90.0f);
		for (uint32_t i = 0; i < amount; i++) {
			newRadian = startRadian + (diffusionRadian * i);
			newVelocity.x = -RadianToVector(newRadian).x * xzLenght;
			newVelocity.y = speedY;
			newVelocity.z = RadianToVector(newRadian).y * xzLenght;
			CreateBullet(transform, newVelocity, type, colliderName, damage, damageCoolTime, i);
		}
	} else {
		startRadian = centerRadian - (diffusionRadian * ((amount - 1) / 2)) + Radian(90.0f);
		for (uint32_t i = 0; i < amount; i++) {
			newRadian = startRadian + (diffusionRadian * i);
			newVelocity.x = -RadianToVector(newRadian).x * xzLenght;
			newVelocity.y = speedY;
			newVelocity.z = RadianToVector(newRadian).y * xzLenght;
			CreateBullet(transform, newVelocity, type, colliderName, damage, damageCoolTime, i);
		}
	}
}

void ProjectileManager::CreateWave(const Transform& transform, float speed, float height, float time, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	std::unique_ptr<Wave> wave;
	wave = std::make_unique<Wave>();
	wave->Initialize(transform, speed, height, time, colliderName, damage, damageCoolTime);
	waves.push_back(std::move(wave));
}

void ProjectileManager::CreateSpike(const Transform& transform, uint32_t size, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	std::unique_ptr<Spike> spike;
	spike = std::make_unique<Spike>();
	spike->Initialize(transform, size, colliderName, damage, damageCoolTime);
	spikes.push_back(std::move(spike));
}

void ProjectileManager::CreateExplode(const Transform& transform, float radius, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	std::unique_ptr<Explode> explode;
	explode = std::make_unique<Explode>();
	explode->Initialize(transform, radius, colliderName, damage, damageCoolTime);
	explodes.push_back(std::move(explode));
}

void ProjectileManager::CreateTutorialObstacle(const Vector3& position,TutorialObstaclesType type){
	std::unique_ptr<TutorialObstacles> tutorial;
	tutorial = std::make_unique<TutorialObstacles>();
	tutorial->Initialize(Transform::GetInitialValue({ 1.0f,1.0f,1.0f },{0.0f,0.0f,0.0f},position), kCollisionEnemyAttack, 0.0f, 1.0f, type);
	tutorials.push_back(std::move(tutorial));
}

void (Bullet::* Bullet::pInitializeFunc[])() = {
		&Bullet::NormalInitialize,
		&Bullet::BounsInitialize,
		&Bullet::SpikeInitialize,
		&Bullet::SlowSpikeInitialize,
};

void (Bullet::* Bullet::pUpdateFunc[])() = {
		&Bullet::NormalUpdate,
		&Bullet::BounsUpdate,
		&Bullet::SpikeUpdate,
		&Bullet::SlowSpikeUpdate,
};

void Bullet::Initialize(const Transform& transform, const Vector3& velocity, BulletType type, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
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
	colliderRadius_ = 0.4f;
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(colliderName);
	colliderDimensionType_ = ColliderDimensionType::k3D;
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("bullet_crystal"));
	model_.SetBlendMode(BlendMode::kNormalCullNone);
	model_.SetLightingType(LightingType::kNone);

	damage_ = damage;
	damageCoolTime_ = damageCoolTime;
	(this->*pInitializeFunc[static_cast<size_t>(type_)])();
}

void Bullet::Update() {
	lifeTimer_ += DeltaTime::GetInstance()->GetGameTime();
	(this->*pUpdateFunc[static_cast<size_t>(type_)])();

	if (lifeTimer_ >= lifeTimeMax_) {
		switch (type_) {
		case BulletType::kBounce:
			if (diffusionAmount_ == 0) {
				SoundManager::GetInstance()->SoundPlay("snd_explode_mini", 1.0f, 0.5f, kSoundEffect);
			}
			ProjectileManager::GetInstance()->CreateExplode(transform_, colliderRadius_ * 2.0f, kCollisionEnemyAttack, 15.0f, 3.0f);
			break;
		default:
			break;
		}
		isActive_ = false;
	}
}

void Bullet::Draw() {
	DrawCollider();
	//Renderer::GetInstance()->DrawSphereWireFrame(modelTransform_, { 1.0f,0.0f,0.0f,1.0f });
	if (type_ != BulletType::kSpike && type_ != BulletType::kSlowSpike) {
		model_.Draw(modelTransform_);
	}
}

void Bullet::NormalInitialize() {
	emitter_ = std::make_unique<Emitter>();
	emitter_->Initialize(transform_, 1, 0.1f);
	emitter_->SetParticle(ParticleManager::GetInstance()->GetParticles("cross"));
	transform_.rotate.y = std::atan2(velocity_.x, velocity_.z);
}

void Bullet::NormalUpdate() {
	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();
	modelTransform_.rotate.x += kModelRotateSpeed * DeltaTime::GetInstance()->GetGameTime();
	modelTransform_.rotate.y += kModelRotateSpeed * DeltaTime::GetInstance()->GetGameTime();
	emitter_->SetTransform(transform_);
	emitter_->Update();
}

void Bullet::BounsInitialize() {
	colliderRadius_ = 0.8f;
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("effect_plane"));
	model_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("bullet_bounce"));
	model_.SetBlendMode(BlendMode::kNormal);
	modelTransform_.Initialize();
	modelTransform_.ClearParent();
	modelTransform_.scale = { 0.8f,0.8f,0.8f };
	//velocity_.y = 1.0f;

	lifeTimeMax_ = 5.0f;
}

void Bullet::BounsUpdate() {
	velocity_.y -= gravityAcceleration_ * DeltaTime::GetInstance()->GetGameTime();
	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();
	colliderDimensionType_ = ColliderDimensionType::k3D;
	if (transform_.GetWorldPosition().y - (colliderRadius_ / 2.0f) <= 0.0f) {
		colliderDimensionType_ = ColliderDimensionType::k2D;
		transform_.translate.y = (colliderRadius_ / 2.0f);

		Vector3 reflected = velocity_.Reflect({ 0.0f,1.0f,0.0f });
		Vector3 projectToNormal = Vector3::Project(reflected, { 0.0f,1.0f,0.0f });
		Vector3 movingDirection = reflected - projectToNormal;
		velocity_ = projectToNormal * kBounsE_ + movingDirection;
	}

	modelTransform_.translate = transform_.GetWorldPosition();
	modelTransform_.rotate = Camera::GetInstance()->GetRotate();
}

void Bullet::SpikeInitialize() {
	transform_.rotate.y = std::atan2(velocity_.x, velocity_.z);
	lifeTimeMax_ = 8.0f;
	spikeCreateTimer_ = 0.0f;
	colliderColor_ = { 1.0f,1.0f,1.0f,1.0f };
	//isColliderActive_ = false;
}

void Bullet::SpikeUpdate() {
	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();
	//transform_.translate.y = 0.0f;
	spikeCreateTimer_ += DeltaTime::GetInstance()->GetGameTime();
	if (spikeCreateTimer_ >= kSpikeCreateRate) {
		spikeCreateTimer_ -= kSpikeCreateRate;
		Transform newTrasform;
		newTrasform = transform_;
		newTrasform.translate += Random::GetInstance()->RandomVector3({ -kRadnomsize_.x / 2.0f,0.0f,-kRadnomsize_.z / 2.0f }, { kRadnomsize_.x / 2.0f,0.0f,kRadnomsize_.z / 2.0f });
		ProjectileManager::GetInstance()->CreateSpike(newTrasform, 0, kCollisionEnemyAttack, damage_, damageCoolTime_);
	}
}

void Bullet::SlowSpikeInitialize(){
	transform_.rotate.y = std::atan2(velocity_.x, velocity_.z);
	spikeRotateY_ = std::atan2(velocity_.x, velocity_.z);
	velocity_.x = 10.0f;
	lifeTimeMax_ = 10.0f;
	spikeCreateTimer_ = 0.0f;
	colliderColor_ = { 1.0f,1.0f,1.0f,1.0f };
	spikeRotatePosX_ = 0.0f;
}

void Bullet::SlowSpikeUpdate(){
	spikeRotatePosX_ += velocity_.x * DeltaTime::GetInstance()->GetGameTime();
	//transform_.translate.y = 0.0f;
	spikeCreateTimer_ += DeltaTime::GetInstance()->GetGameTime();
	spikeRotateY_ += kSpikeRotateSpeed * DeltaTime::GetInstance()->GetGameTime();
	std::vector<Vector3> spikeCreatePos;
	for (uint32_t i = 0; i < 4; i++) {
		Vector3 pos = { 0.0f,0.5f,0.0f };
		pos.x = Rotate({ spikeRotatePosX_,0.0f }, { transform_.GetWorldPosition().x,transform_.GetWorldPosition().z }, (i * 90.0f) + Degree(spikeRotateY_)).x;
		pos.z = Rotate({ spikeRotatePosX_,0.0f }, { transform_.GetWorldPosition().x,transform_.GetWorldPosition().z }, (i * 90.0f) + Degree(spikeRotateY_)).y;
		spikeCreatePos.push_back(pos);
	}

	if (spikeCreateTimer_ >= kSpikeCreateRate) {
		spikeCreateTimer_ -= kSpikeCreateRate;
		Transform newTrasform;
		for (Vector3& pos : spikeCreatePos) {
			newTrasform.Initialize();
			newTrasform.translate = pos;
			newTrasform.rotate.y = spikeRotateY_;
			ProjectileManager::GetInstance()->CreateSpike(newTrasform, 0, kCollisionEnemyAttack, damage_, damageCoolTime_);
		}
	}
}

void Wave::Initialize(const Transform& transform, float speed, float height, float time, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	heightMax_ = height;
	height_ = heightMax_;
	transform_.scale = { 1.0f,1000.0f,1.0f };
	colliderMinorRadius_ = heightMax_ * 0.001f;
	transform_.rotate = { 0.0f,0.0f,0.0f };
	transform_.translate = transform.GetWorldPosition();
	transform_.translate.y = 0.0f;
	colliderRadius_ = 0.0f;
	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(colliderName);
	speed_ = speed;
	isActive_ = true;
	lifeTimer_ = 0.0f;
	colliderDimensionType_ = ColliderDimensionType::kAll;
	colliderType_ = ColliderType::kTorus;
	if (time == -1.0f) {
		lifeTimeMax_ = kBasicLifeTimeMax_;
		isTimeInf_ = true;
	} else {
		lifeTimeMax_ = time;
		isTimeInf_ = false;
	}

	damage_ = damage;
	damageCoolTime_ = damageCoolTime;
}

void Wave::Update() {
	lifeTimer_ += DeltaTime::GetInstance()->GetGameTime();
	colliderRadius_ += speed_ * DeltaTime::GetInstance()->GetGameTime();

	if (!isTimeInf_) {
		height_ = Easing(heightMax_, 0.0f, lifeTimer_, lifeTimeMax_, EaseType::kEaseIn);
		colliderMinorRadius_ = height_ * 0.001f;
		//transform_.scale = { 1.0f,Easing(1000.0f, 1.0f, lifeTimer_, lifeTimeMax_, EaseType::kEaseIn) / heightMax_,1.0f };
	}

	if (lifeTimer_ >= lifeTimeMax_) {
		isActive_ = false;
	}
}

void Wave::Draw() {
	Renderer::GetInstance()->DrawTorus(transform_, colliderRadius_, colliderMinorRadius_, "white_template", { 1.0f,0.7f,0.7f,1.0f });
}

void Spike::Initialize(const Transform& transform, uint32_t size, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	uint32_t sizeIndex;
	sizeIndex = size;
	if (size == 0) {
		sizeIndex = static_cast<uint32_t>(Random::GetInstance()->RandomFloat(1.0f, 3.0f));
	}


	transform_.Initialize();
	transform_.translate = transform.translate;
	transform_.translate.y = -(colliderSize_.y / 2.0f);

	modelTransform_.Initialize();
	modelTransform_.SetParent(&transform_);
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("spike"));
	switch (sizeIndex) {
	case 1:
		colliderSize_ = kBasicSpikeSize;
		modelTransform_.scale = { 1.0f,1.0f,1.0f };
		break;
	case 2:
		colliderSize_ = kBasicSpikeSize * 1.5f;
		modelTransform_.scale = { 1.5f,1.5f,1.5f };
		break;
	case 3:
		colliderSize_ = kBasicSpikeSize * 2.0f;
		modelTransform_.scale = { 2.0f,2.0f,2.0f };
		break;
	}
	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(colliderName);
	isActive_ = true;
	lifeTimer_ = 0.0f;
	colliderType_ = ColliderType::kBox;
	colliderDimensionType_ = ColliderDimensionType::kAll;
	spikePhase_ = 0;
	lifeTimeMax_ = kStartTimeMax;
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };

	damage_ = damage;
	damageCoolTime_ = damageCoolTime;
}

void Spike::Update() {
	lifeTimer_ += DeltaTime::GetInstance()->GetGameTime();
	switch (spikePhase_) {
	case 0:
		transform_.translate.y = Easing(-colliderSize_.y, colliderSize_.y, lifeTimer_, lifeTimeMax_, EaseType::kEaseOut);
		if (lifeTimer_ >= lifeTimeMax_) {
			lifeTimer_ = 0;
			spikePhase_++;
			lifeTimeMax_ = kStayTimeMax;
		}
		break;
	case 1:

		if (lifeTimer_ >= lifeTimeMax_) {
			lifeTimer_ = 0;
			spikePhase_++;
			lifeTimeMax_ = kEndTimeMax;
		}
		break;
	case 2:
		transform_.translate.y = Easing(colliderSize_.y, -colliderSize_.y, lifeTimer_, lifeTimeMax_, EaseType::kEaseIn);

		if (lifeTimer_ >= lifeTimeMax_) {
			lifeTimer_ = 0;
			spikePhase_++;
			isActive_ = false;
		}
		break;
	}

	DrawCollider();
}

void Spike::Draw() {
	//Renderer::GetInstance()->DrawBoxWireFrame(GetOBB(), { 1.0f,0.0f,0.0f,1.0f });
	model_.Draw(modelTransform_);
}

void Explode::Initialize(const Transform& transform, float radius, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	transform_.Initialize();
	transform_.translate = transform.GetWorldPosition();
	transform_.translate.y = 0.0f;
	maxColliderRadius_ = radius;
	colliderRadius_ = 0.0f;
	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(colliderName);
	isActive_ = true;
	lifeTimer_ = 0.0f;
	colliderType_ = ColliderType::kSphere;
	colliderDimensionType_ = ColliderDimensionType::kAll;
	lifeTimeMax_ = kStartTimeMax;
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };

	for (uint32_t i = 0; i < 30; i++) {
		ParticleManager::GetInstance()->SpawnParticles("explode", transform_.GetWorldPosition());
	}

	phase_ = 0;

	damage_ = damage;
	damageCoolTime_ = damageCoolTime;
}

void Explode::Update() {
	lifeTimer_ += DeltaTime::GetInstance()->GetGameTime();

	switch (phase_) {
	case 0:
		colliderRadius_ = Easing(0.0f, maxColliderRadius_, lifeTimer_, lifeTimeMax_, EaseType::kEaseIn);

		if (lifeTimer_ >= lifeTimeMax_) {
			lifeTimer_ = 0;
			phase_++;
			lifeTimeMax_ = kStayTimeMax;
		}
		break;
	case 1:
		colliderRadius_ = maxColliderRadius_;
		if (lifeTimer_ >= lifeTimeMax_) {
			lifeTimer_ = 0;
			isActive_ = false;
		}
		break;
	default:
		break;
	}

	DrawCollider();
}

void TutorialObstacles::Initialize(const Transform& transform, CollisionAttributeName colliderName, float damage, float damageCoolTime, TutorialObstaclesType type){
	type_ = type;

	transform_.Initialize();
	transform_.translate = transform.translate;


	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(colliderName);
	isActive_ = true;
	colliderType_ = ColliderType::kBox;
	colliderDimensionType_ = ColliderDimensionType::kAll;
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
	modelTransform_.Initialize();
	modelTransform_.SetParent(&transform_);
	switch (type_){
	case TutorialObstaclesType::kBullet:
		colliderRadius_ = 0.8f;
		colliderType_ = ColliderType::kSphere;
		colliderDimensionType_ = ColliderDimensionType::k3D;
		model_.Initialize(ModelManager::GetInstance()->GetModelInfo("bullet_crystal"));
		model_.SetBlendMode(BlendMode::kNormalCullNone);
		model_.SetLightingType(LightingType::kNone);
		break;
	case TutorialObstaclesType::kSpike:
		colliderSize_ = kBasicSpikeSize;
		modelTransform_.scale = { 1.5f,1.5f,1.5f } ;
		transform_.translate.y = (colliderSize_.y / 2.0f) * 1.5f;
		model_.Initialize(ModelManager::GetInstance()->GetModelInfo("spike"));
		break;
	}



	damage_ = damage;
	damageCoolTime_ = damageCoolTime;
}

void TutorialObstacles::Update() {
	switch (type_){
	case TutorialObstaclesType::kBullet:
		modelTransform_.rotate.x += kModelRotateSpeed * DeltaTime::GetInstance()->GetGameTime();
		modelTransform_.rotate.y += kModelRotateSpeed * DeltaTime::GetInstance()->GetGameTime();
		break;
	}

}

void TutorialObstacles::Draw() {
	DrawCollider();
	model_.Draw(modelTransform_,true);
}