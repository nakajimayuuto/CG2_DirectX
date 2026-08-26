#include "MirrorHalberd.h"
#include "ProjectileManager.h"

void MirrorHalberd::Initialize(HalberdName name) {
	name_ = name;
	halberdModel_.Initialize("halberd");
	halberdModel_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("halberd_soul"));
	transform_.Initialize();
	transform_.translate = { 0.0f,0.0f,0.0f };
	modelTransform_.Initialize();
	modelTransform_.SetParent(&transform_);
	SetSize({ 0.6f,2.8f,0.8f });
	SetRadius(3.5f);
	SetColliderType(ColliderType::kBox);
	SetDimensionType(ColliderDimensionType::k3D);
	//SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack));
	//SetCollisionMask(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer));

	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy);
	collisionMask_ =
		CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack) |
		CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer);
	colliderColor_ = { 0.6f,0.3f,1.0f,1.0f };

	//SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	SetDamage(10.0f);
	SetDamageCoolTime(3.0f);
	isColliderActive_ = false;

	maxHP_ = 200.0f;
	currentHP_ = maxHP_;

	hpGauge = std::make_unique<HPGauge>();
	hpGauge->Initialize(&currentHP_, maxHP_, { 350.0f,15.0f });
	hpGauge->SetPosition({ 0.0f,-150.0f });
	switch (name_) {
	case MirrorHalberd::HalberdName::kLeft:
		hpGauge->SetPosition({ -210.0f,-180.0f });
		break;
	case MirrorHalberd::HalberdName::kRight:
		hpGauge->SetPosition({ 210.0f,-180.0f });
		break;
	}

	nameScaleX_ = 0.0f;
	scaleTimer_ = 0.0f;
	hpGauge->SetScale({ nameScaleX_, 1.0f });

	isActive_ = false;
	isDeath_ = false;
	isAutoMove_ = false;
	isFinished_ = false;
}

void MirrorHalberd::Update() {
	//SetActive(isActive_);
	if (!isActive_) {
		return;
	}

	hpGauge->Update();


	if (isAutoMove_) {


		if (!isAttack_) {
			damageCountFirst_ = 0;
			damageCountSecond_ = 0;
			damageCountThird_ = 0;
		}

		if (damageCoolTimer_ > 0.0f) {
			damageCoolTimer_ -= DeltaTime::GetInstance()->GetGameTime();
			if (damageCoolTimer_ <= 0.0f) {
				damageCoolTimer_ = 0.0f;
			}
		}

		if (scaleTimer_ >= kScaleTimer) {
			scaleTimer_ = kScaleTimer;
		} else {
			scaleTimer_ += DeltaTime::GetInstance()->GetGameTime();
		}

		AttackUpdate();
	} else {
		if (scaleTimer_ <= 0.0f) {
			scaleTimer_ = 0.0f;
		} else {
			scaleTimer_ -= DeltaTime::GetInstance()->GetGameTime();
		}
	}

	nameScaleX_ = Easing(0.0f,1.0f,scaleTimer_,kScaleTimer,EaseType::kEaseInOut);

	hpGauge->SetScale({ nameScaleX_, 1.0f });

	CollisionManager::GetInstance()->AddColliderList(this);
}

void MirrorHalberd::Draw() {
	if (!isActive_) {
		return;
	}
	DrawCollider();

	Renderer* renderer = Renderer::GetInstance();
	if (!isDeath_) {
		renderer->DrawModel(modelTransform_, &halberdModel_, true);
		renderer->DrawShadow(modelTransform_, &halberdModel_, { 0.0f,0.0f,0.0f,1.0f });
	}

	//if (isAutoMove_) {
		hpGauge->Draw();
		switch (name_){
		case MirrorHalberd::HalberdName::kLeft:
			renderer->DrawSprite(Transform::GetInitialValue({ nameScaleX_,1.0f,1.0f }, {0.0f,0.0f,0.0f}, { -210.0f,-220.0f,0.0f }), "name_left_halberd", { 1.0f,1.0f,1.0f,1.0f });
			break;
		case MirrorHalberd::HalberdName::kRight:
			renderer->DrawSprite(Transform::GetInitialValue({ nameScaleX_,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 210.0f,-220.0f,0.0f }), "name_right_halberd", { 1.0f,1.0f,1.0f,1.0f });
			break;
		}
	//}
}

void MirrorHalberd::AutoStart() {
	transform_.translate = transform_.GetWorldPosition();
	//transform_.translate.y;
	transform_.ClearParent();
	currentPhase_ = 0;
	currentAttack_ = HalberdAttack::kWaveShot;
	isAutoMove_ = true;
	isFinished_ = false;
	isDeath_ = false;
	currentAttackTimer_ = 0.0f;
	kMaxAttackTimer = kStartTimerMax;
	AttackInitialize();
}

void MirrorHalberd::AttackInitialize() {
	isColliderActive_ = false;
	moveTransform_.Initialize();
	preTransform_ = transform_;
	float directionY = atan2(targetTransform_->translate.x - bossTransform_->translate.x, targetTransform_->translate.z - bossTransform_->translate.z);

	if (name_ == HalberdName::kLeft) {
		moveTransform_.translate = targetTransform_->translate + Matrix4x4::MakeRotateYMatrix(directionY).TransformNomal({ 5.0f,0.0f,-20.0f });//Random::GetInstance()->RandomVector3({-1.0f,-1.0f,-1.0f}, {1.0f,1.0f,1.0f}).Normalize() * 10.0f;
	} else {
		moveTransform_.translate = targetTransform_->translate + Matrix4x4::MakeRotateYMatrix(directionY).TransformNomal({ -5.0f,0.0f,-20.0f });//Random::GetInstance()->RandomVector3({-1.0f,-1.0f,-1.0f}, {1.0f,1.0f,1.0f}).Normalize() * 10.0f;
	}


	moveTransform_.translate.y = basicPosY;
	moveTransform_.rotate.y = atan2(moveTransform_.translate.x - targetTransform_->translate.x, moveTransform_.translate.z - targetTransform_->translate.z);

	currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	kMaxAttackTimer = 1.0f; // 攻撃のタイマー最大値.
	currentAttackPhase = 0; // 攻撃のフェーズ.
	SetAttack();

	switch (currentAttack_)
	{
	case MirrorHalberd::HalberdAttack::kFangShot:
		FangAttackInitialize();
		break;
	case MirrorHalberd::HalberdAttack::kWaveShot:
		WaveInitialize();
		break;
	case MirrorHalberd::HalberdAttack::kBulletShot:
		BulletInitialize();
		break;
	case MirrorHalberd::HalberdAttack::kDiffusionShot:
		DiffusionBulletInitialize();
		break;
	default:
		break;
	}
}

void MirrorHalberd::AttackUpdate() {
	currentAttackTimer_ += gameTime_;
	switch (currentPhase_) {
	case 0:
		transform_.translate = Easing(preTransform_.translate, moveTransform_.translate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate = Easing(preTransform_.rotate, moveTransform_.rotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			currentAttackTimer_ = 0.0f;
			kMaxAttackTimer = 1.0f;
			currentPhase_++;
			isColliderActive_ = true;
		}
		break;
	case 1:
		switch (currentAttack_) {
		case MirrorHalberd::HalberdAttack::kFangShot:
			FangAttackUpdate();
			break;
		case MirrorHalberd::HalberdAttack::kWaveShot:
			WaveUpdate();
			break;
		case MirrorHalberd::HalberdAttack::kBulletShot:
			BulletUpdate();
			break;
		case MirrorHalberd::HalberdAttack::kDiffusionShot:
			DiffusionBulletUpdate();
			break;
		}
		break;
	case 2:
		if (currentAttackTimer_ >= kAttackGapTimerMax) {
			currentPhase_ = 0;
			AttackInitialize();
		}

		if (isFinished_) {
			moveTransform_.Initialize();
			preTransform_ = transform_; 
			currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
			kMaxAttackTimer = kFinsihGapTimerMax; // 攻撃のタイマー最大値.
			currentAttackPhase = 0; // 攻撃のフェーズ.
			currentPhase_ = 4;
		}
		break;
	case 3:
		RespawnUpdate();
		if (currentHP_ >= maxHP_) {
			currentPhase_ = 0;
			isDeath_ = false;

			AttackInitialize();
		}

		if (isFinished_) {
			AutoAttackStop();
		}
		break;
	case 4:
		transform_.translate = Easing(preTransform_.translate, bossTransform_->translate, currentAttackTimer_,kMaxAttackTimer,EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AutoAttackStop();
		}
		break;
	}
}

void MirrorHalberd::AttackFinished() {
	currentPhase_++;
	currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	kMaxAttackTimer = 0.0f; // 攻撃のタイマー最大値.
	currentAttackPhase = 0; // 攻撃のフェーズ.
}

void MirrorHalberd::WaveInitialize() {
	kMaxAttackTimer = kWaveStartGapTimerMax;

}

void MirrorHalberd::WaveUpdate() {

	switch (currentAttackPhase) {
	case 0: // ハルバードを上昇.
		transform_.translate.y = Easing(basicPosY, kWaveHalberdStayPos.y, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate = Easing({ 0.0f,0.0f,0.0f }, kWaveHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(0.0f, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveStayTimerMax);
		}
		break;
	case 1: // ハルバードを上昇.
		modelTransform_.rotate = Easing(kWaveHalberdStartRotate, kWaveHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveAttackTimerMax);
		}
		break;
	case 2: // 攻撃態勢に入りながら急降下.
		transform_.translate.y = Easing(kWaveHalberdStayPos.y, kWaveHalberdAttackPos.y, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveAttackGapTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateWave(transform_, 25.0f, 1.0f, -1.0f, kCollisionEnemyAttack, 15.0f, 3.0f);
		}
		break;
	case 3: // 攻撃後の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveFinishedGapTimerMax);
		}
		break;
	case 4: // 見た目を戻す.
		transform_.translate.y = Easing(kWaveHalberdAttackPos.y, basicPosY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		modelTransform_.rotate = Easing(kWaveHalberdAttackPos, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			modelTransform_.rotate.y = 0.0f;
		}
		break;
	}
}

void MirrorHalberd::FangAttackInitialize() {
	//AttackFinished();

	kMaxAttackTimer = kFangAttackStartGapTimerMax;
}

void MirrorHalberd::FangAttackUpdate() {
	Transform effectTransform;
	switch (currentAttackPhase) {
	case 0: // 上昇しながらハルバードを前に構える.
		transform_.translate.y = Easing(basicPosY, kFangAttackAnimPositionY / 2.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		modelTransform_.rotate = Easing({ 0.0f,0.0f,0.0f }, kFangAttackHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kFangAttackStartGapTimerMax);
		}
		break;
	case 1: // 残りの上昇.
		transform_.translate.y = Easing(kFangAttackAnimPositionY / 2.0f, kFangAttackAnimPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.x = Easing(kFangAttackHalberdStartRotate.x, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kFangAttackSpinTimerMax);
		}
		break;
	case 2: // その場で回転.
		transform_.rotate.x = Easing(0.0f, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kFangAttackAttackTimerMax);
		}
		break;
	case 3: // 攻撃態勢に入りながら急降下.
		transform_.translate.y = Easing(kFangAttackAnimPositionY, kFangAttackAttackPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		modelTransform_.rotate.x = Easing(0.0f, kFangAttackAttackRotateX, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f, 0.25f, kSoundEffect);
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 0.25f, kSoundEffect);
			NextAttackPhase(kFangAttackAttackGapTimerMax);
			FangAttackFangCreate();

		}
		break;
	case 4: // 攻撃後の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kFangAttackFinishedGapTimerMax);
		}
		break;
	case 5: // 見た目を戻す.
		transform_.translate.y = Easing(kFangAttackAttackPositionY, basicPosY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		modelTransform_.rotate.x = Easing(kFangAttackAttackRotateX, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void MirrorHalberd::FangAttackFangCreate() {
	float rotateY = transform_.rotate.y - Radian(90.0f);
	float lenght = Vector3(targetTransform_->GetWorldPosition() - transform_.GetWorldPosition()).Length();
	Transform newTransform;
	newTransform.Initialize();
	Vector2 center = { transform_.GetWorldPosition().x, transform_.GetWorldPosition().z };

	ProjectileManager::GetInstance()->CreateBullet(transform_, Vector3(-RadianToVector(rotateY).x, 0.0f, RadianToVector(rotateY).y) * 20.0f, BulletType::kSpike, kCollisionEnemyAttack, 15.0f, 3.0f);
}

void MirrorHalberd::BulletInitialize() {
	kMaxAttackTimer = kBulletStartGapTimerMax;
	bulletShotDirectionTemp_ = { 0.0f,0.0f,1.0f };
}

void MirrorHalberd::BulletUpdate() {
	switch (currentAttackPhase) {
	case 0: // ハルバードを前に向ける.
		modelTransform_.rotate = Easing({ 0.0f,0.0f,0.0f }, kBasicHalberdFarRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBulletStayTimerMax);
			if ((targetTransform_->translate - transform_.GetWorldPosition()).Length() != 0.0f) {
				bulletShotDirectionTemp_ = GetBulletDire();
			}
		}
		break;
	case 1: // 攻撃の前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBulletAttackGapTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateBullet(transform_, bulletShotDirectionTemp_ * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f);
		}
		break;
	case 2: // 攻撃の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBulletFinishedGapTimerMax);
		}
		break;
	case 3: // 見た目を戻す.
		modelTransform_.rotate = Easing(kBasicHalberdFarRotate, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void MirrorHalberd::DiffusionBulletInitialize() {
	kMaxAttackTimer = kDiffusionBulletStartGapTimerMax;
}

void MirrorHalberd::DiffusionBulletUpdate() {
	switch (currentAttackPhase) {
	case 0: // ハルバードを前に構える.
		modelTransform_.rotate = Easing({ 0.0f,0.0f,0.0f }, kDiffusionBulletHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletSpinTimerMax);
		}
		break;
	case 1: // ハルバードを高速回転させる.
		modelTransform_.rotate = Easing(kDiffusionBulletHalberdStartRotate, kDiffusionBulletHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletBackTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateDiffusionBullet(transform_, GetBulletDire() * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(15.0f), 3);

		}
		break;
	case 2: // 少し後退
		modelTransform_.translate.z = Easing(0.0f, kDiffusionBulletAnimPositionZ, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletBackTimerMax);
		}
		break;
	case 3: // 元の位置に戻る.
		modelTransform_.translate.z = Easing(kDiffusionBulletAnimPositionZ, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletAttackGapTimerMax);
		}
		break;
	case 4: // 後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletFinishedGapTimerMax);
		}
		break;
	case 5: // 見た目を戻す.
		modelTransform_.rotate = Easing(kDiffusionBulletHalberdStartRotate, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void MirrorHalberd::SetAttack() {
	float random = Random::GetInstance()->RandomFloat(0.0f, 4.0f);
	if (random < 1.0f) {
		currentAttack_ = HalberdAttack::kBulletShot;
	} else if (random < 2.0f) {
		currentAttack_ = HalberdAttack::kDiffusionShot;
	} else if (random < 3.0f) {
		currentAttack_ = HalberdAttack::kWaveShot;
	} else {
		currentAttack_ = HalberdAttack::kFangShot;
	}
}

void MirrorHalberd::RespawnInitialize() {
	if (isFinished_) {
		transform_.translate = bossTransform_->translate;
		currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
		kMaxAttackTimer = kFinsihGapTimerMax; // 攻撃のタイマー最大値.
		currentAttackPhase = 0; // 攻撃のフェーズ.
		AutoAttackStop();
		for (uint32_t i = 0; i < 30; i++) {
			ParticleManager::GetInstance()->SpawnParticles("death_cross", transform_.GetWorldPosition());
		}
		return;
	}

	currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	kMaxAttackTimer = kRespawnStartGapTimerMax; // 攻撃のタイマー最大値.
	currentAttackPhase = 0; // 攻撃のフェーズ.
	for (uint32_t i = 0; i < 30; i++) {
		ParticleManager::GetInstance()->SpawnParticles("death_cross", transform_.GetWorldPosition());
	}

	currentPhase_ = 3;
	isColliderActive_ = false;
	isDeath_ = true;
}

void MirrorHalberd::RespawnUpdate() {
	switch (currentAttackPhase) {
	case 0: // ハルバードを前に構える.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletSpinTimerMax);
		}
		break;
	case 1:
		for (uint32_t i = 0; i < 5; i++) {
			ParticleManager::GetInstance()->SpawnParticles("charge", transform_.GetWorldPosition());
		}
		currentHP_ += gameTime_ * kRespawnHPIncreese;
		if (currentHP_ >= maxHP_ / 2.0f) {
			currentAttackPhase++;
		}
		break;
	case 2:
		currentHP_ += gameTime_ * kRespawnHPIncreese;
		break;
	}
}

void MirrorHalberd::AutoAttackStop() {
	modelTransform_.Initialize();
	modelTransform_.SetParent(&transform_);
	transform_.Initialize();
	transform_.SetParent(bossTransform_);
	isDeath_ = false;
	isAutoMove_ = false;
	isColliderActive_ = false;
	currentAttackTimer_ = 0.0f;
}

void MirrorHalberd::OnCollision(Collider* other) {
	if (isDeath_) {
		return;
	}

	if (other->GetCollisionAttribute() == CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer)) {
		return;
	}

	//if ((other->GetCollisionAttribute() & CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack)) == 0x0) {
	switch (other->GetDamageType()) {
	case 1:
		if (damageCountFirst_ <= 0) {
			currentHP_ -= other->GetDamage();

			DeltaTime::GetInstance()->SetHitStop(0.05f);

			damageCountFirst_++;
		}
		break;
	case 2:
		if (damageCountSecond_ <= 1) {
			if (damageCoolTimer_ <= 0.0f) {
				currentHP_ -= other->GetDamage();

				DeltaTime::GetInstance()->SetHitStop(0.05f);

				damageCoolTimer_ = other->GetDamageCoolTime();

				damageCountSecond_++;
			}
		}
		break;
	case 3:
		if (damageCountThird_ <= 4) {
			if (damageCoolTimer_ <= 0.0f) {
				currentHP_ -= other->GetDamage();
				DeltaTime::GetInstance()->SetHitStop(0.05f);

				damageCoolTimer_ = other->GetDamageCoolTime();
				damageCountThird_++;
			}
		}
		break;
	default:
		if (damageCoolTimer_ <= 0.0f) {
			currentHP_ -= other->GetDamage();

			DeltaTime::GetInstance()->SetHitStop(0.05f);

			damageCoolTimer_ = other->GetDamageCoolTime();
		}
		break;
	}

	if (currentHP_ <= 0.0f) {

		DeltaTime::GetInstance()->SetHitStop(0.5f);
		RespawnInitialize();
	}
}