#include "ProjectileManager.h"
#include "MapChipManager.h"

ProjectileManager* ProjectileManager::GetInstance() {
	static ProjectileManager instance;
	return &instance;
}

void ProjectileManager::Initialize() {
	bullets.clear();
	waves.clear();
	spikes.clear();
	explodes.clear();
	lightnings.clear();
	meteors.clear();

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

	for (auto& lightning : lightnings) {
		lightning->Update();
#ifndef _DEBUG
		if (currentLightNum_ < kBulletLightMax_) {

			if (lightning->GetPhase() == 0) {
				LightManager::GetInstance()->GetLightData(lightNames_[currentLightNum_])->color = { 1.0f,0.0f,0.0f,1.0f };
			} else {
				LightManager::GetInstance()->GetLightData(lightNames_[currentLightNum_])->color = { 0.5f,0.5f,1.0f,1.0f };
			}

			LightManager::GetInstance()->SetLightPos(lightNames_[currentLightNum_], lightning->GetTranasform().GetWorldPosition());
			LightManager::GetInstance()->GetLightData(lightNames_[currentLightNum_])->intensity = 10.0f * lightning->GetAlpha();
			LightManager::GetInstance()->GetLightData(lightNames_[currentLightNum_])->radius = 2.0f;
			LightManager::GetInstance()->SetLightIsActive(lightNames_[currentLightNum_], true);
			currentLightNum_++;
		}
#endif // _DEBUG
	}

	for (auto it = lightnings.begin(); it != lightnings.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = lightnings.erase(it);
		} else {
			++it;
		}
	}

	for (auto& lightning : lightnings) {
		CollisionManager::GetInstance()->AddColliderList(lightning->GetCollider());
	}

	for (auto& meteor : meteors) {
		meteor->Update();
	}

	for (auto it = meteors.begin(); it != meteors.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = meteors.erase(it);
		} else {
			++it;
		}
	}

	for (auto& meteor : meteors) {
		CollisionManager::GetInstance()->AddColliderList(reinterpret_cast<Collider*>(meteor.get()));
	}

	for (auto& enemyBlock : enemyBlocks) {
		enemyBlock->Update();
	}

	for (auto it = enemyBlocks.begin(); it != enemyBlocks.end(); ) {
		if (!(*it)->GetIsActive()) {
			it = enemyBlocks.erase(it);
		} else {
			++it;
		}
	}

	for (auto& enemyBlock : enemyBlocks) {
		CollisionManager::GetInstance()->AddColliderList(reinterpret_cast<Collider*>(enemyBlock.get()));
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

	for (auto& lightning : lightnings) {
		lightning->Draw();
	}

	for (auto& meteor : meteors) {
		meteor->Draw();
	}

	for (auto& enemyBlock : enemyBlocks) {
		enemyBlock->Draw();
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

void ProjectileManager::CreateLightning(const Transform& transform, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	std::unique_ptr<Lightning> lightning;
	lightning = std::make_unique<Lightning>();
	lightning->Initialize(transform, colliderName, damage, damageCoolTime);
	lightnings.push_back(std::move(lightning));
}

void ProjectileManager::CreateMeteor(const Transform& transform, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	std::unique_ptr<Meteor> meteor;
	meteor = std::make_unique<Meteor>();
	meteor->Initialize(transform, colliderName, damage, damageCoolTime);
	meteors.push_back(std::move(meteor));
}

void ProjectileManager::CreateEnemyBlock(const Transform& transform, const Transform& targetTransform, CollisionAttributeName colliderName, float damage, uint32_t type) {
	std::unique_ptr<EnemyBlock> enemyBlock;
	enemyBlock = std::make_unique<EnemyBlock>();
	enemyBlock->Initialize(transform, targetTransform, colliderName, damage, static_cast<float>(type));
	enemyBlocks.push_back(std::move(enemyBlock));
}

void ProjectileManager::CreateTutorialObstacle(const Vector3& position, TutorialObstaclesType type) {
	std::unique_ptr<TutorialObstacles> tutorial;
	tutorial = std::make_unique<TutorialObstacles>();
	tutorial->Initialize(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, position), kCollisionEnemyAttack, 0.0f, 1.0f, type);
	tutorials.push_back(std::move(tutorial));
}

void (Bullet::* Bullet::pInitializeFunc[])() = {
		&Bullet::NormalInitialize,
		&Bullet::BounsInitialize,
		&Bullet::SpikeInitialize,
		&Bullet::SlowSpikeInitialize,
		&Bullet::BlockInitialize,
};

void (Bullet::* Bullet::pUpdateFunc[])() = {
		&Bullet::NormalUpdate,
		&Bullet::BounsUpdate,
		&Bullet::SpikeUpdate,
		&Bullet::SlowSpikeUpdate,
		&Bullet::BlockUpdate,
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

void Bullet::SlowSpikeInitialize() {
	transform_.rotate.y = std::atan2(velocity_.x, velocity_.z);
	spikeRotateY_ = std::atan2(velocity_.x, velocity_.z);
	velocity_.x = 10.0f;
	lifeTimeMax_ = 10.0f;
	spikeCreateTimer_ = 0.0f;
	colliderColor_ = { 1.0f,1.0f,1.0f,1.0f };
	spikeRotatePosX_ = 0.0f;
}

void Bullet::SlowSpikeUpdate() {
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

void Bullet::BlockInitialize() {
	isColliderActive_ = true;
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("block_template"));
	model_.SetBlendMode(BlendMode::kNormal);
	model_.SetLightingType(LightingType::kHalfLambert);
	lifeTimeMax_ = 20.0f;
	modelTransform_.rotate.y = std::atan2(velocity_.x, velocity_.z);
	switch (static_cast<uint32_t>(damageCoolTime_)) {
	case 3:
		model_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("soul_block"));
		damage_ = damage_ * 3.0f;
		break;
	case 4:
		model_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("explode_block"));
		break;
	}
}

void Bullet::BlockUpdate() {
	velocity_.y += -kBlockGravityAccelerationY * DeltaTime::GetInstance()->GetGameTime();
	//modelTransform_.rotate.x += kModelRotateSpeed * DeltaTime::GetInstance()->GetGameTime();
	//modelTransform_.rotate.y += kModelRotateSpeed * DeltaTime::GetInstance()->GetGameTime();

	if (!isColliderActive_) {
		isActive_ = false;
		return;
	}

	CollisionMapInfo collisionMapInfo;

	collisionMapInfo.movementAmount = velocity_ * DeltaTime::GetInstance()->GetGameTime();
	collisionMapInfo.width = kBlockWidth;
	collisionMapInfo.height = kBlockHeight;

	//MapChipField::IndexSet indexSet;
	//indexSet = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipIndexSetByPosition(transform_.GetWorldPosition() + collisionMapInfo.movementAmount);
	//MapChipType type = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	//uint32_t subID = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipSubIDByIndex(indexSet.xIndex, indexSet.yIndex);
	MapChipManager::GetInstance()->MapCollision(transform_.translate, collisionMapInfo);

	if (
		collisionMapInfo.isCellingCollision ||
		collisionMapInfo.isLanding ||
		collisionMapInfo.isWallCollision
		) {
		if (collisionMapInfo.isLanding) {
			MapChipManager::GetInstance()->CreateBlock(transform_.translate, static_cast<uint32_t>(damageCoolTime_));
			isActive_ = false;
			return;
		} else {
			if (collisionMapInfo.subID != 2 && collisionMapInfo.type == MapChipType::kBlock) {
				MapChipManager::GetInstance()->CreateBlock(transform_.translate, static_cast<uint32_t>(damageCoolTime_));
				isActive_ = false;
			}
			collisionMapInfo.movementAmount.x = 0.0f;
			velocity_.x = 0.0f;
			if (collisionMapInfo.isCellingCollision) {
				collisionMapInfo.movementAmount.y = 0.0f;
				velocity_.y = 0.0f;
			}
			return;
		}
	}

	transform_.translate += collisionMapInfo.movementAmount;

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

void TutorialObstacles::Initialize(const Transform& transform, CollisionAttributeName colliderName, float damage, float damageCoolTime, TutorialObstaclesType type) {
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
	switch (type_) {
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
		modelTransform_.scale = { 1.5f,1.5f,1.5f };
		transform_.translate.y = (colliderSize_.y / 2.0f) * 1.5f;
		model_.Initialize(ModelManager::GetInstance()->GetModelInfo("spike"));
		break;
	}



	damage_ = damage;
	damageCoolTime_ = damageCoolTime;
}

void TutorialObstacles::Update() {
	switch (type_) {
	case TutorialObstaclesType::kBullet:
		modelTransform_.rotate.x += kModelRotateSpeed * DeltaTime::GetInstance()->GetGameTime();
		modelTransform_.rotate.y += kModelRotateSpeed * DeltaTime::GetInstance()->GetGameTime();
		break;
	}

}

void TutorialObstacles::Draw() {
	DrawCollider();
	model_.Draw(modelTransform_, true);
}

void Lightning::Initialize(const Transform& transform, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	transform_ = transform;
	transform_.rotate = { 0.0f,0.0f,0.0f };
	MapChipField::IndexSet indexSet;
	indexSet = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipIndexSetByPosition(transform.translate);
	uint32_t yIndex = 0;
	while (true) {
		MapChipType type = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipTypeByIndex(indexSet.xIndex, yIndex);
		if (type == MapChipType::kBlock) {
			break;
		}
		yIndex++;
	}
	transform_.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(indexSet.xIndex, yIndex - 1);
	collider_ = std::make_unique<Collider>();
	collider_->SetDamage(damage);
	collider_->SetDamageCoolTime(damageCoolTime);
	model_.Initialize("lightning");
	model_.SetLightingType(LightingType::kNone);
	warningBlock_.Initialize("block_template");
	warningBlock_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("skeleton_block_warning"));
	warningBlock_.SetColor({ 1.0f,0.0f,0.0f,1.0f });
	collider_->SetSize({ 0.5f, 20.0f,0.5f });
	collider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	collider_->SetColliderType(ColliderType::kBox);
	collider_->SetActive(false);
	collider_->SetRadius(20.0f);
	collider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(colliderName));
	isActive_ = true;
	isColliderActive_ = false;

	lifeTimer_ = 0.0f;
	lifeTimeMax_ = 1.0f;

	warningTimer_ = 0.0f;

	warningCount_ = 0;
	warningCountMax_ = kBasicWarningCountMax_;

	phase_ = 0;
	alpha_ = 1.0f;
	isColorRed_ = true;
}

void Lightning::Update() {
	Transform colliderTransform = transform_;
	colliderTransform.translate.y = transform_.translate.y + 20.0f;
	collider_->SetTransform(colliderTransform);
	switch (phase_) {
	case 0:
		warningTimer_ += DeltaTime::GetInstance()->GetGameTime();

		if (warningTimer_ >= kWarningGlowTimeMax_) {
			if (warningCount_ >= warningCountMax_) {
				phase_ = 1;
				transform_.translate.y -= 1.5f;
				collider_->SetActive(true);
				MapChipManager::GetInstance()->DeleteBlock(transform_.GetWorldPosition(), 0, 0);
			}

			if (isColorRed_) {
				isColorRed_ = false;
				warningBlock_.SetColor({ 1.0f,0.0f,0.0f,1.0f });
			} else {
				isColorRed_ = true;
				warningBlock_.SetColor({ 1.0f,1.0f,0.0f,1.0f });
			}

			warningCount_++;
			warningTimer_ -= kWarningGlowTimeMax_;
		}
		break;
	case 1:
		lifeTimer_ += DeltaTime::GetInstance()->GetGameTime();
		alpha_ = Easing(1.0f, 0.0f, lifeTimer_, lifeTimeMax_, EaseType::kConstant);
		model_.SetColor(Vector4::MakeColorWithoutAlpha({ 1.0f,1.0f,1.0f }, alpha_));
		if (lifeTimer_ >= lifeTimeMax_) {
			isActive_ = false;
		}
		break;
	}

	collider_->DrawCollider();
}

void Lightning::Draw() {
	switch (phase_) {
	case 0:
		warningBlock_.Draw(transform_);
		break;
	case 1:
		model_.Draw(transform_);
		break;
	}
}

void Meteor::Initialize(const Transform& transform, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	transform_ = transform;
	transform_.translate.y = 30.0f;
	transform_.rotate = { 0.0f,0.0f,0.0f };
	transform_.scale = { 3.0f,3.0f,1.0f };
	damage_ = damage;
	damageCoolTime_ = damageCoolTime;
	model_.Initialize("block_template");
	model_.SetLightingType(LightingType::kNone);
	warningBlock_.Initialize("block_template");
	warningBlock_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("skeleton_block"));
	warningBlock_.SetColor({ 1.0f,0.0f,0.0f,1.0f });
	colliderSize_ = { 1.5f,1.5f,1.0f };
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
	colliderType_ = ColliderType::kBox;
	isColliderActive_ = true;
	colliderRadius_ = 20.0f;
	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(colliderName);
	isActive_ = true;
	isColliderActive_ = true;

	lifeTimer_ = 0.0f;
	lifeTimeMax_ = 10.0f;

	warningTimer_ = 0.0f;

	warningBlockTransform_.scale = { 3.0f,3.0f,1.0f };
	warningCount_ = 0;
	warningCountMax_ = kBasicWarningCountMax_;

	hitCount_ = 0;
	MapChipField::IndexSet indexSet;
	Vector3 newPos = transform_.translate;
	newPos.y = 35.0f;
	indexSet = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipIndexSetByPosition(newPos);
	transform_.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(indexSet.xIndex, indexSet.yIndex);

	phase_ = 0;
	alpha_ = 1.0f;
	isColorRed_ = true;

	hitStopTimer_ = 0.0f;
	hitStopTimeMax_ = 0.2f;
}

void Meteor::Update() {
	MapChipField::IndexSet indexSet;
	Vector3 newPos = transform_.translate;
	newPos.y = 35.0f;
	indexSet = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipIndexSetByPosition(newPos);
	uint32_t yIndex = 0;
	uint32_t count = 0;
	bool isExplode = false;
	while (true) {
		MapChipType type;
		uint32_t subId;
		for (uint32_t i = 0; i < 3; i++) {
			type = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipTypeByIndex(indexSet.xIndex - 1 + i, yIndex);
			subId = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipSubIDByIndex(indexSet.xIndex - 1 + i, yIndex);
			if (type == MapChipType::kBlock) {
				count++;
				break;
			}
		}

		if (subId == 2) {
			isExplode = true;
			yIndex--;
			break;
		}

		if (count >= kHitCountMax_ - hitCount_) {
			break;
		}
		yIndex++;
	}
	warningBlockTransform_.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(indexSet.xIndex, yIndex - 1);

	if (!isColliderActive_) {
		isActive_ = false;
		return;
	}

	switch (phase_) {
	case 0:
		if (isExplode) {
			warningBlock_.SetColor({ 1.0f,0.0f,0.0f,1.0f });
		} else {
			warningBlock_.SetColor({ 0.5f,0.5f,1.0f,1.0f });
		}

		warningTimer_ += DeltaTime::GetInstance()->GetGameTime();
		if (warningTimer_ >= kWarningTimeMax_) {
			phase_++;
		}
		break;
	case 1:
		if (isExplode) {
			warningBlock_.SetColor({ 1.0f,0.0f,0.0f,1.0f });
		} else {
			warningBlock_.SetColor({ 0.5f,0.5f,1.0f,1.0f });
		}

		if (hitStopTimer_ > 0.0f) {
			hitStopTimer_ -= DeltaTime::GetInstance()->GetGameTime();
			break;
		}
		lifeTimer_ += DeltaTime::GetInstance()->GetGameTime();
		Vector3 collisionPosition = transform_.GetWorldPosition();
		collisionPosition.x -= 1.0f;
		collisionPosition.y -= 1.0f;
		CollisionMapInfo info;
		info.width = 0.9f;
		info.height = 0.9f;
		info.movementAmount = { 0.0f,0.0f,0.0f };
		info.movementAmount.y = -kFallingSpeed * DeltaTime::GetInstance()->GetGameTime();
		for (uint32_t i = 0; i < 3; i++) {
			MapChipManager::GetInstance()->MapCollisionNoCenter(collisionPosition, info);
			if (info.isLanding) {
				if (hitCount_ >= kHitCountMax_) {
					collisionPosition = transform_.GetWorldPosition();
					collisionPosition.y = transform_.GetWorldPosition().y - 1.0f;
					for (uint32_t i = 0; i < 3; i++) {
						collisionPosition.x = transform_.GetWorldPosition().x - 1.0f;
						for (uint32_t j = 0; j < 3; j++) {
							MapChipManager::GetInstance()->CreateBlock(collisionPosition);
							collisionPosition.x += 1.0f;
						}
						collisionPosition.y += 1.0f;
					}
					isActive_ = false;
					break;
				}
				if (info.subID == 2) {
					isActive_ = false;
					break;
				}
				MapChipManager::GetInstance()->DeleteBlock(transform_.GetWorldPosition(), -1, -2);
				MapChipManager::GetInstance()->DeleteBlock(transform_.GetWorldPosition(), 0, -2);
				MapChipManager::GetInstance()->DeleteBlock(transform_.GetWorldPosition(), 1, -2);
				hitStopTimer_ = hitStopTimeMax_;
				hitCount_++;
				break;
			}
			collisionPosition.x += 1.0f;
		}

		transform_.translate += info.movementAmount;


		//alpha_ = Easing(1.0f, 0.0f, lifeTimer_, lifeTimeMax_, EaseType::kConstant);
		//model_.SetColor(Vector4::MakeColorWithoutAlpha({ 1.0f,1.0f,1.0f }, alpha_));
		if (lifeTimer_ >= lifeTimeMax_) {
			isActive_ = false;
		}
		if (transform_.translate.y <= -4.0f) {
			isActive_ = false;
		}
		break;
	}

	DrawCollider();
}

void Meteor::Draw() {
	warningBlock_.Draw(warningBlockTransform_);
	model_.Draw(transform_);
	switch (phase_) {
	case 0:
		break;
	case 1:
		break;
	}
}

void EnemyBlock::Initialize(const Transform& transform, const Transform& targetTransform, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	InitializeTemplate(transform, colliderName, damage, damageCoolTime);
	//anchorPoint_.push_back(transform.GetWorldPosition() + (Vector3(0.3f, -1.0f, 0.0f) * (transform.GetWorldPosition() - targetTransform.GetWorldPosition()).Length()));
	//anchorPoint_.push_back(transform.GetWorldPosition());
	anchorPoint_.push_back(transform.GetWorldPosition());
	Vector3 centerPosition = ((transform.GetWorldPosition() - targetTransform.GetWorldPosition()) / 2.0f) + targetTransform.GetWorldPosition();
	centerPosition.y = 30.0f;
	anchorPoint_.push_back(centerPosition);
	anchorPoint_.push_back(targetTransform.GetWorldPosition());
	float length = (transform.GetWorldPosition() - centerPosition).Length() + (centerPosition - targetTransform.GetWorldPosition()).Length();
	anchorPoint_.push_back(targetTransform.GetWorldPosition() + (Vector3(0.0f, -1.0f, 0.0f) * length));
	type = kBlockToPlayer;
	moveTimer_ = 0.0f;
}

void EnemyBlock::Initialize(const Transform& transform, uint32_t mapIndexX, CollisionAttributeName colliderName, float damage, float damageCoolTime)
{
	InitializeTemplate(transform, colliderName, damage, damageCoolTime);
	//anchorPoint_.push_back(transform.GetWorldPosition() + (Vector3(0.3f, -1.0f, 0.0f) * (transform.GetWorldPosition() - targetTransform.GetWorldPosition()).Length()));
	//anchorPoint_.push_back(transform.GetWorldPosition());
	anchorPoint_.push_back(transform.GetWorldPosition());
	MapChipField::IndexSet indexSet;
	indexSet = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipIndexSetByPosition(transform.translate);
	uint32_t yIndex = 0;
	while (true) {
		MapChipType type = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipTypeByIndex(indexSet.xIndex, yIndex);
		if (type == MapChipType::kBlock) {
			break;
		}
		yIndex++;
	}
	Vector3 finalPos = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(mapIndexX, yIndex);
	Vector3 centerPosition = ((transform.GetWorldPosition() - finalPos) / 2.0f) + finalPos;
	centerPosition.y = 30.0f;
	anchorPoint_.push_back(centerPosition);
	anchorPoint_.push_back(finalPos);
	float length = (transform.GetWorldPosition() - centerPosition).Length() + (centerPosition - finalPos).Length();
	anchorPoint_.push_back(finalPos + (Vector3(0.0f, -1.0f, 0.0f) * length));
	type = kBlockToField;
	moveTimer_ = 0.0f;
}

void EnemyBlock::Update() {
	if (!isColliderActive_) {
		isActive_ = false;
		return;
	}

	switch (type) {
	case EnemyBlock::kBlockToPlayer:
	case EnemyBlock::kBlockToField:
		switch (phase_) {
		case 0:
			moveTimer_ += DeltaTime::GetInstance()->GetGameTime();
			transform_.translate = CatmullRomInterpolation(anchorPoint_, moveTimer_, kMoveTimerMax_);
			moveAmount_ = (prePos_ - transform_.translate);
			if (moveTimer_ >= kMoveTimerMax_) {
				isActive_ = false;
			}
			break;
		}
		prePos_ = transform_.translate;

		CollisionMapInfo collisionMapInfo;

		collisionMapInfo.movementAmount = { 0.0f,-1.0f,0.0f };
		collisionMapInfo.width = 0.0f;
		collisionMapInfo.height = 0.0f;

		MapChipManager::GetInstance()->MapCollision(prePos_, collisionMapInfo);

		if (collisionMapInfo.isLanding) {
			MapChipManager::GetInstance()->CreateBlock(prePos_, static_cast<uint32_t>(damageCoolTime_));
			isActive_ = false;
			return;
		}
	}


	DrawCollider();
}

void EnemyBlock::Draw() {
	model_.Draw(transform_);
	if (phase_ == 2) {
	}
}

void EnemyBlock::InitializeTemplate(const Transform& transform, CollisionAttributeName colliderName, float damage, float damageCoolTime) {
	transform_ = transform;
	transform_.translate.y = 30.0f;
	transform_.rotate = { 0.0f,0.0f,0.0f };
	transform_.scale = { 1.0f,1.0f,1.0f };
	model_.Initialize("block_template");
	model_.SetLightingType(LightingType::kNone);


	damage_ = damage;
	damageCoolTime_ = damageCoolTime;
	colliderSize_ = {0.4f,0.4f,0.4f};
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };
	colliderType_ = ColliderType::kBox;
	isColliderActive_ = true;
	colliderRadius_ = 20.0f;
	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(colliderName);

	isActive_ = true;
	isColliderActive_ = true;

	lifeTimer_ = 0.0f;
	phase_ = 0;
}
