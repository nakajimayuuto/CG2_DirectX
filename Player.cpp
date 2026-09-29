#include "Player.h"
#define NOMINMAX
#include <algorithm>
#include "GameCamera.h"
void Player::Initialize() {
	transform_.Initialize();
	targetRotateY = 0.0f;
	model_.Initialize("drill_ghost");
	model_.SetBlendMode(BlendMode::kNormalCullNone);
	//transform_.translate = { 0.0f, kTranslateBlankY,-30.0f };
	transform_.translate = { 5.0f, kTranslateBlankY + 1.0f,0.0f };

	/// HPGauge.
	maxHP_ = 200.0f;
	currentHP_ = maxHP_;

	hpGauge_ = std::make_unique<HPGauge>();
	hpGauge_->Initialize(&currentHP_, maxHP_, { 200.0f,20.0f });
	hpGauge_->SetPosition({ -500.0f,300.0f });

	// 後々削除
	transformColliderOffset.Initialize();
	transformColliderOffset.translate.y = kBodyBlankY;
	transformColliderOffset.SetParent(&transform_);

	transformModel.Initialize();
	transformModel.SetParent(&transformColliderOffset);
	//transform_.rotate.y = std::atan2(velocity.x, velocity.z);
	//Vector3 velocityXZ = { velocity.x,0.0f,velocity.z };
	//transform_.rotate.x = std::atan2(-velocity.y, velocityXZ.Length());

	behavior_ = Behavior::kRoot;

	InitializeFloatingGimmick();
	colliderDimensionType_ = ColliderDimensionType::k3D;
	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer);
	collisionMask_ = (
		CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy) |
		CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack)
		);

	//BehaviorAttackInitialize();

	DifficultyManager::GetInstance()->SetPlayerHPData(&currentHP_, maxHP_);

	//emitter_ = std::make_unique<Emitter>();
	//particles_ = std::make_unique<Particles>();
	//particles_->Initialize(TextureManager::GetInstance()->GetTextureInfo("effect_plane"));
	//particles_->SetBillboardType(BillboardType::kAllAxis);
	//emitter_->SetParticle(particles_.get());
	//emitter_->Initialize(transform_, 3, 0.5f);

	colliderRadius_ = 2.0f;
	colliderSize_ = { 0.5f,1.0f,0.5f };
	colliderType_ = ColliderType::kBox;


	GameCamera::GetInstance()->SetTarget(&transform_);

	LightManager::GetInstance()->CreatePointLight("player_light");
	LightManager::GetInstance()->GetLightData("player_light")->color = { 0.5f,0.5f,1.0f,1.0f };
	LightManager::GetInstance()->GetLightData("player_light")->radius = 5.0f;
	LightManager::GetInstance()->GetLightData("player_light")->intensity = 1.0f;

	tutorialTimer_ = 0.0f;

	isImmune_ = false;

	isDeath_ = false;

	deathTimer_ = 0.0f;
	onGround_ = true;
}

void Player::InitializeFloatingGimmick() {
	floatingParameter = 0.0f;
}

bool Player::GetAttackButtonTrigger() {
	InputManager* input = InputManager::GetInstance();
	if (input->IsGamePadConnect()) {
		if (input->TriggerPadButton(PadButtons::INPUT_X) || input->TriggerPadButton(PadButtons::INPUT_Y)) {
			return true;
		}
	} else {
		if (input->TriggerMouse(MouseButtons::MOUSE_LEFT)) {
			return true;
		}
	}

	return false;
}

bool Player::GetJumpButtonTrigger() {
	InputManager* input = InputManager::GetInstance();
	if (input->IsGamePadConnect()) {
		if (input->TriggerPadButton(PadButtons::INPUT_A) || input->TriggerPadButton(PadButtons::INPUT_B)) {
			return true;
		}
	} else {
		if (input->TriggerKey(DIK_SPACE)) {
			return true;
		}
	}

	return false;
}

bool Player::GetDownPress() {
	InputManager* input = InputManager::GetInstance();
	if (input->IsGamePadConnect()) {
		if (input->GetLeftStickDirection().y <= -0.5f) {
			return true;
		}
	} else {
		if (input->PressKey(DIK_S)) {
			return true;
		}
	}

	return false;
}

float Player::GetDirectionYPress() {
	InputManager* input = InputManager::GetInstance();
	if (input->IsGamePadConnect()) {
		//if (input->GetLeftStickDirection().y <= - 0.5f) {
		return input->GetLeftStickDirection().y;
		//}
	} else {
		Vector2 direction = { 0.0f,0.0f };
		if (input->PressKey(DIK_W)) {
			direction.y += 1.0f;
		}

		if (input->PressKey(DIK_A)) {
			direction.x -= 1.0f;
		}

		if (input->PressKey(DIK_S)) {
			direction.y -= 1.0f;
		}

		if (input->PressKey(DIK_D)) {
			direction.x += 1.0f;
		}

		return VectorToRadian(direction);
	}

}

void Player::FloatingAccelerationChange() {
	return;

	InputManager* input = InputManager::GetInstance();
	Vector3 acceleration = { 0.0f,0.0f,0.0f };
	if (input->IsGamePadConnect()) {
		Vector3 acceleration = { input->GetLeftStickDirection().x,0.0f,input->GetLeftStickDirection().y };

		acceleration = acceleration.Normalize() * kFloatingAcceleration;
	} else {
		if (input->PressKey(DIK_W)) {
			acceleration.z += 1.0f;
		}

		if (input->PressKey(DIK_S)) {
			acceleration.z -= 1.0f;
		}

		if (input->PressKey(DIK_A)) {
			acceleration.x -= 1.0f;
		}

		if (input->PressKey(DIK_D)) {
			acceleration.x += 1.0f;
		}

		acceleration = acceleration.Normalize() * kFloatingAcceleration;
	}

	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateMatrix(Camera::GetInstance()->GetTransform().rotate);

	acceleration = cameraRotateMatrix.TransformNomal(acceleration);

	velocity_ += acceleration * DeltaTime::GetInstance()->GetGameTime();
}

void Player::Update() {
#ifdef _DEBUG
	ImGui::Begin("player");
	ImGui::DragFloat("HP", &currentHP_, 1.0f, 0.0f, maxHP_);
	ImGui::Text("pos %f,%f,%f", transform_.translate.x, transform_.translate.y, transform_.translate.z);
	ImGui::Text("Behavior %s", magic_enum::enum_name(behavior_).data());
	ImGui::End();
#endif // _DEBUG
	LightManager::GetInstance()->SetLightPos("player_light", transform_.GetWorldPosition());
	LightManager::GetInstance()->SetLightIsActive("player_light", true);

	CheckTutorialFlag();

	if (behaviorRequest_) {
		behavior_ = behaviorRequest_.value();

		switch (behavior_) {
		case Player::Behavior::kRoot:
			BehaviorRootInitialize();
			break;
		case Player::Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		case Player::Behavior::kDash:
			BehaviorDashInitialize();
			break;
		case Player::Behavior::kJump:
			BehaviorJumpInitialize();
			break;
		case Player::Behavior::kDashAttack:
			BehaviorDashAttackInitialize();
			break;
		case Player::Behavior::kDashJumpAttack:
			BehaviorDashJumpAttackInitialize();
			break;
		case Player::Behavior::kFall:
			BehaviorFallInitialize();
			break;
		}

		behaviorRequest_ = std::nullopt;
	}

	isAttack_ = false;
	isDash_ = false;
	isDash_ = false;

	colliderDimensionType_ = ColliderDimensionType::k3D;
	switch (behavior_) {
	case Player::Behavior::kRoot:
		BehaviorRootUpdate();
		CollisionManager::GetInstance()->AddColliderList(this);
		break;
	case Player::Behavior::kAttack:
		BehaviorAttackUpdate();
		CollisionManager::GetInstance()->AddColliderList(this);
		break;
	case Player::Behavior::kDash:
		BehaviorDashUpdate();
		colliderDimensionType_ = ColliderDimensionType::k2D;
		CollisionManager::GetInstance()->AddColliderList(this);
		LightManager::GetInstance()->SetLightIsActive("player_light", false);
		break;
	case Player::Behavior::kJump:
		BehaviorJumpUpdate();
		CollisionManager::GetInstance()->AddColliderList(this);
		break;
	case Player::Behavior::kDashAttack:
		BehaviorDashAttackUpdate();
		CollisionManager::GetInstance()->AddColliderList(this);
		break;
	case Player::Behavior::kDashJumpAttack:
		BehaviorDashJumpAttackUpdate();
		CollisionManager::GetInstance()->AddColliderList(this);
		break;
	case Player::Behavior::kFall:
		BehaviorFallUpdate();
		CollisionManager::GetInstance()->AddColliderList(this);
		break;
	}

	CheckTutorialUpdate();

	colliderColor_ = { 0.5f,0.5f,1.0f,1.0f };

	if (damageCoolTimer_ > 0.0f) {
		damageCoolTimer_ -= DeltaTime::GetInstance()->GetGameTime();
		if (damageCoolTimer_ <= 0.0f) {
			damageCoolTimer_ = 0.0f;
		}
	}

	transform_.rotate.y = std::fmod(transform_.rotate.y, Radian(360.0f));

	//if (gGamePhase == GamePhase::kTutorial || gGamePhase == GamePhase::kBossLastJarona || gGamePhase == GamePhase::kGameClearStage) {

		//TutorialWallClamp();

	CircleWallClamp();


	GameCamera::GetInstance()->SetTargetIsMove(isMoving_);
	GameCamera::GetInstance()->SetTargetIsDash(isDash_);
}

void Player::CircleWallClamp() {
	/// ここから地獄
	float distance = transform_.translate.Length();
	float posY = transform_.translate.y;
	float wallDirection = 0.0f;
	if (distance > movingRadius_) {
		transform_.translate = transform_.translate.Normalize() * movingRadius_;
		transform_.translate.y = posY;
	}
}

void Player::TutorialWallClamp() {
	if (transform_.translate.x > 5.0f - colliderSize_.x) {
		transform_.translate.x = 5.0f - colliderSize_.x;
	}

	if (transform_.translate.x < -5.0f + colliderSize_.x) {
		transform_.translate.x = -5.0f + colliderSize_.x;
	}

	if (transform_.translate.z > tutorialClampMinPosZ_ - colliderSize_.z) {
		transform_.translate.z = tutorialClampMinPosZ_ - colliderSize_.z;
	}

	if (transform_.translate.z < tutorialClampMaxPosZ_ + colliderSize_.z) {
		transform_.translate.z = tutorialClampMaxPosZ_ + colliderSize_.z;
	}
}

void Player::SlashEffectCreate(Transform* targetTransform, uint32_t num) {
	Transform effectCreate;
	for (uint32_t i = 0; i < 3; i++) {
		effectCreate.Initialize();
		effectCreate.SetParent(targetTransform);
		effectCreate.translate = Random::GetInstance()->RandomVector3(-(*targetTransform).scale / 2.0f, (*targetTransform).scale / 2.0f);

		ParticleManager::GetInstance()->SpawnParticles("cross", effectCreate.GetWorldPosition());
	}
}

void Player::BehaviorRootInitialize() {
	floatingParameter = 0.0f;

	InitializeFloatingGimmick();
}



void Player::BehaviorRootUpdate() {
	if (isDeath_) {
		return;
	}

	//if (InputManager::GetInstance()->TriggerKey(DIK_SPACE)) {
	//	behaviorRequest_ = Behavior::kAttack;
	//}

	MovingUpdate();

	MapCollisionUpdate();

	UpdateFloatingGimmick();

}

void Player::MovingUpdate() {
	if (InputManager::GetInstance()->PressKey(DIK_RIGHT) || InputManager::GetInstance()->PressKey(DIK_LEFT)) {
		// 左右加速.
		Vector3 acceleration = {};

		if (InputManager::GetInstance()->PressKey(DIK_RIGHT)) {
			// 左移動中の右入力.
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}

			acceleration.x += kAcceleration;

			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;

				turnFirstRotationY_ = transform_.rotate.y;
				turnTimer_ = kTimeTurn;
			}
		} else if (InputManager::GetInstance()->PressKey(DIK_LEFT)) {
			// 右移動中の左入力.
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}

			acceleration.x -= kAcceleration;

			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;

				turnFirstRotationY_ = transform_.rotate.y;
				turnTimer_ = kTimeTurn;
			}
		}

		// 加速減速.
		velocity_ += acceleration;

		// 最大速度制限.
		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
	} else {
		// 非入力時は移動減衰をかける.
		if (velocity_.x <= 0.01f && velocity_.x >= -0.01f) {
			velocity_.x = 0.0f;
		} else {
			velocity_.x *= (1.0f - kAttenuation);
		}
	}

	if (onGround_) {
		if (InputManager::GetInstance()->PressKey(DIK_UP)) {
			//behaviorRequest_ = Behavior::kJump;
			velocity_ += Vector3(0.0f, kJumpAcceleration, 0.0f);
		}
	} else {
		// 落下速度.
		velocity_ += Vector3(0.0f, -kGravityAcceleration, 0.0f);
		// 速度制限.
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

void Player::CollisionMoveUpdate(const CollisionMapInfo& info) {
	transform_.translate += static_cast<Vector3>(info.movementAmount);
}

void Player::CellingCollisionUpdate(const CollisionMapInfo& info) {
	if (info.isCellingCollision) {
		GameSystem::GetInstance()->Log("hit ceiling\n");
		velocity_.y = 0.0f;
	}
}

void Player::IsHitWallUpdate(const CollisionMapInfo& info) {
	if (info.isWallCollision) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::IsGroundUpdate(const CollisionMapInfo& info) {
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			if (!MapChipManager::GetInstance()->OnGroundCheck(transform_.translate, info)) {
				onGround_ = false;
			}
		}
	} else {
		if (info.isLanding) {
			onGround_ = true;
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;
			//behaviorRequest_ = Behavior::kRoot;
		}
	}
}

void Player::TurningControl() {
	// 旋回制御.

	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル.
		float destinationRotationYTable[] = {
			Radian(90.0f),
			Radian(270.0f),
		};

		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		transform_.rotate.y = Easing(turnFirstRotationY_, destinationRotationY, kTimeTurn - turnTimer_, kTimeTurn, EaseType::kConstant);
	}
}

void Player::CheckFallVoid() {
	if (transform_.translate.y <= -2.0f) {
		isDeath_ = true;
	}
}

void Player::MapCollisionUpdate() {
	CollisionMapInfo collisionMapInfo;

	collisionMapInfo.movementAmount = velocity_ * DeltaTime::GetInstance()->GetGameTime();

	MapChipManager::GetInstance()->MapCollision(transform_.translate, collisionMapInfo);

	CollisionMoveUpdate(collisionMapInfo);

	CellingCollisionUpdate(collisionMapInfo);

	IsHitWallUpdate(collisionMapInfo);

	IsGroundUpdate(collisionMapInfo);

	TurningControl();

	CheckFallVoid();
}

void Player::BehaviorJumpInitialize() {
	velocity_.y = kJumpFirstSpeed_;
	SoundManager::GetInstance()->SoundPlay("snd_step", 1.0f, 1.0f, kSoundEffect);


}

void Player::BehaviorJumpUpdate() {
	isJump_ = true;

	FloatingAccelerationChange();

	Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };
	velocity_ += accelerationVector * DeltaTime::GetInstance()->GetGameTime();
	//transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();


	if (transform_.translate.y <= kTranslateBlankY) {
		transform_.translate.y = kTranslateBlankY;
		behaviorRequest_ = Behavior::kRoot;
		SoundManager::GetInstance()->SoundPlay("snd_step", 1.0f, 1.0f, kSoundEffect);
	}

	if (tutorialUsableDash_) {
		if (GetAttackButtonTrigger()) {
			SoundManager::GetInstance()->SoundPlay("snd_shine", 1.0f, 1.0f, kSoundEffect);
			behaviorRequest_ = Behavior::kDashAttack;
		}
	}

	MapCollisionUpdate();

	UpdateFloatingGimmick();
}

void Player::BehaviorAttackInitialize() {
	transform_.rotate.y = targetRotateY;
	attackTimer_ = 0.0f;
	attackTimeMax_ = kAttackFirstStart;
	attackComboPhase_ = 0;
	attackPhase_ = 0;
	attackTransform_.Initialize();
	attackTransform_.SetParent(&transformColliderOffset);
	transformModel.SetParent(&attackTransform_);
	AttackFirstInitialize();
}

void Player::BehaviorAttackUpdate() {
	isAttack_ = true;
	attackTimer_ += DeltaTime::GetInstance()->GetGameTime();


	switch (attackComboPhase_) {
	case 0:
		AttackFirstUpdate();
		break;
	case 1:
		AttackSecondUpdate();
		break;
	case 2:
		AttackThreeUpdate();
		break;
	}

}

void Player::BehaviorAttackFinished() {
	behaviorRequest_ = Behavior::kRoot;
	transformModel.SetParent(&transformColliderOffset);
}

void Player::CheckTutorialFlag() {
	tutorialUsableMove_ = true;
	tutorialUsableJump_ = true;
	tutorialUsableDash_ = true;
	tutorialUsableAttack_ = true;

}

void Player::CheckTutorialUpdate() {
}

void Player::SetNextAttackPhase(float timeMax) {
	attackTimeMax_ = timeMax;
	attackPhase_++;
	attackTimer_ = 0.0f;
}

void Player::AttackFirstInitialize() {
	useNextAttack_ = false;
	attackCollider_.SetRadius(4.5f);
	attackCollider_.SetDamage(35.0f);
	attackCollider_.SetDamageCoolTime(0.1f);
	attackCollider_.SetDamageType(1);
	attackCollider_.SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack));
	SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 1.0f, kSoundEffect);
}

void Player::AttackFirstUpdate() {
	attackCollider_.SetActive(false);
	attackCollider_.SetTransform(transform_);
	attackCollider_.SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	Transform effectTransform;
	effectTransform.Initialize();
	effectTransform.SetParent(&transformModel);
	effectTransform.scale = { 1.0f,1.0f,1.0f };
	effectTransform.translate = { 0.0f,2.0f,0.0f };
	SlashEffectCreate(&effectTransform, 3);
	switch (attackPhase_) {
	case 0:
		attackTransform_.rotate.x = Easing(0.0f, kAttackFirstStartModelRotateX, attackTimer_, attackTimeMax_, EaseType::kEaseIn);
		transformModel.translate.y = Easing(0.0f, kAttackFirstSpinModelPosY, attackTimer_, attackTimeMax_, EaseType::kEaseIn);

		if (attackTimer_ >= attackTimeMax_) {
			SetNextAttackPhase(kAttackFirstSpin);
		}
		break;
	case 1:
		attackCollider_.SetActive(true);
		attackTransform_.rotate.x = Easing(kAttackFirstStartModelRotateX, kAttackFirstSpinModelRotateX, attackTimer_, attackTimeMax_, EaseType::kConstant);

		if (!useNextAttack_) {
			if (GetAttackButtonTrigger()) {
				useNextAttack_ = true;
			}
		}

		if (attackTimer_ >= attackTimeMax_) {
			SetNextAttackPhase(kAttackFirstFinish);
		}
		break;
	case 2:
		attackTransform_.rotate.x = Easing(kAttackFirstSpinModelRotateX, Radian(360.0f), attackTimer_, attackTimeMax_, EaseType::kEaseOut);
		transformModel.translate.y = Easing(kAttackFirstSpinModelPosY, 0.0f, attackTimer_, attackTimeMax_, EaseType::kEaseOut);

		if (!useNextAttack_) {
			if (GetAttackButtonTrigger()) {
				useNextAttack_ = true;
			}
		}

		if (attackTimer_ >= attackTimeMax_) {
			if (useNextAttack_) {
				attackComboPhase_++;
				AttackSecondInitialize();
			} else {
				BehaviorAttackFinished();
			}
		}
		break;
	}

	CollisionManager::GetInstance()->AddColliderList(&attackCollider_);
	attackCollider_.DrawCollider();
}

void Player::AttackSecondInitialize() {
	useNextAttack_ = false;
	attackTimer_ = 0.0f;
	attackTimeMax_ = kAttackSecondStart;
	attackPhase_ = 0;
	attackTransform_.Initialize();
	attackTransform_.SetParent(&transformColliderOffset);
	transformModel.SetParent(&attackTransform_);
	attackCollider_.SetRadius(4.5f);
	attackCollider_.SetDamage(30.0f);
	attackCollider_.SetDamageCoolTime(0.05f);
	attackCollider_.SetDamageType(2);
	attackCollider_.SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack));
	SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 1.0f, kSoundEffect);
}

void Player::AttackSecondUpdate() {
	attackCollider_.SetActive(false);
	attackCollider_.SetTransform(transform_);
	attackCollider_.SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	Transform effectTransform;
	effectTransform.Initialize();
	effectTransform.SetParent(&transformModel);
	effectTransform.scale = { 1.0f,1.0f,1.0f };
	effectTransform.translate = { 0.0f,2.0f,0.0f };
	SlashEffectCreate(&effectTransform, 3);
	switch (attackPhase_) {
	case 0:
		attackTransform_.rotate.x = Easing(0.0f, kAttackFirstStartModelRotateX, attackTimer_, attackTimeMax_, EaseType::kEaseIn);
		transformModel.translate.y = Easing(0.0f, kAttackFirstSpinModelPosY, attackTimer_, attackTimeMax_, EaseType::kEaseIn);

		if (attackTimer_ >= attackTimeMax_) {
			SetNextAttackPhase(kAttackSecondSpin);
		}
		break;
	case 1:
		attackCollider_.SetActive(true);
		attackTransform_.rotate.x = Easing(kAttackFirstStartModelRotateX, kAttackFirstSpinModelRotateX, attackTimer_, attackTimeMax_, EaseType::kConstant);

		if (!useNextAttack_) {
			if (GetAttackButtonTrigger()) {
				useNextAttack_ = true;
			}
		}

		if (attackTimer_ >= attackTimeMax_) {
			SetNextAttackPhase(kAttackSecondFinish);
		}
		break;
	case 2:
		attackTransform_.rotate.x = Easing(kAttackFirstSpinModelRotateX, Radian(360.0f), attackTimer_, attackTimeMax_, EaseType::kEaseOut);
		transformModel.translate.y = Easing(kAttackFirstSpinModelPosY, 0.0f, attackTimer_, attackTimeMax_, EaseType::kEaseOut);

		if (!useNextAttack_) {
			if (GetAttackButtonTrigger()) {
				useNextAttack_ = true;
			}
		}

		if (attackTimer_ >= attackTimeMax_) {
			if (useNextAttack_) {
				attackComboPhase_++;
				AttackThreeInitialize();
			} else {
				BehaviorAttackFinished();
			}
		}
		break;
	}

	CollisionManager::GetInstance()->AddColliderList(&attackCollider_);
	attackCollider_.DrawCollider();
}

void Player::AttackThreeInitialize() {
	useNextAttack_ = false;
	attackTimer_ = 0.0f;
	attackTimeMax_ = kAttackThirdStart;
	attackPhase_ = 0;
	attackTransform_.Initialize();
	attackTransform_.SetParent(&transformColliderOffset);
	transformModel.SetParent(&attackTransform_);
	attackCollider_.SetRadius(5.5f);
	attackCollider_.SetDamage(25.0f);
	attackCollider_.SetDamageCoolTime(0.02f);
	attackCollider_.SetDamageType(3);
	attackCollider_.SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack));
	SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f, 1.0f, kSoundEffect);
}

void Player::AttackThreeUpdate() {
	attackCollider_.SetActive(false);
	attackCollider_.SetTransform(transform_);
	attackCollider_.SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	Transform effectTransform;
	effectTransform.Initialize();
	effectTransform.SetParent(&transformModel);
	effectTransform.scale = { 1.0f,2.0f,1.0f };
	effectTransform.translate = { 0.0f,2.5f,0.0f };
	SlashEffectCreate(&effectTransform, 3);
	switch (attackPhase_) {
	case 0:
		attackTransform_.rotate.x = Easing(0.0f, kAttackFirstStartModelRotateX, attackTimer_, attackTimeMax_, EaseType::kEaseIn);
		transformModel.translate.y = Easing(0.0f, kAttackFirstSpinModelPosY, attackTimer_, attackTimeMax_, EaseType::kEaseIn);

		if (attackTimer_ >= attackTimeMax_) {
			SetNextAttackPhase(kAttackThirdSpin);
		}
		break;
	case 1:
		attackCollider_.SetActive(true);
		attackTransform_.rotate.x = Easing(kAttackFirstStartModelRotateX, kAttackFirstSpinModelRotateX, attackTimer_, attackTimeMax_, EaseType::kConstant);

		if (attackTimer_ >= attackTimeMax_) {
			SetNextAttackPhase(kAttackThirdFinish);
		}
		break;
	case 2:
		attackTransform_.rotate.x = Easing(kAttackFirstSpinModelRotateX, Radian(360.0f), attackTimer_, attackTimeMax_, EaseType::kEaseOut);
		transformModel.translate.y = Easing(kAttackFirstSpinModelPosY, 0.0f, attackTimer_, attackTimeMax_, EaseType::kEaseOut);

		if (attackTimer_ >= attackTimeMax_) {
			BehaviorAttackFinished();
		}
		break;
	}

	CollisionManager::GetInstance()->AddColliderList(&attackCollider_);
	attackCollider_.DrawCollider();
}

void Player::BehaviorDashInitialize() {
	//transform_.rotate.y = targetRotateY;
}

void Player::BehaviorDashUpdate() {
	isDash_ = true;
	isMoving_ = true;
	InputManager* input = InputManager::GetInstance();
	if (input->IsGamePadConnect()) {
		if (tutorialUsableJump_) {
			if (GetJumpButtonTrigger()) {
				behaviorRequest_ = Behavior::kDashJumpAttack;
			}
		}

		if (tutorialUsableMove_) {
			transform_.rotate.y += input->GetLeftStickDirection().x * Radian(3.0f);
		}
	} else {
		if (tutorialUsableMove_) {
			if (input->PressKey(DIK_A)) {
				transform_.rotate.y -= Radian(3.0f);
			}

			if (input->PressKey(DIK_D)) {
				transform_.rotate.y += Radian(3.0f);
			}
		}

		if (tutorialUsableJump_) {
			if (GetJumpButtonTrigger()) {
				behaviorRequest_ = Behavior::kDashJumpAttack;
			}
		}
	}

	Vector3 move = { 0.0f,0.0f,kDashSpeed };

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(transform_.rotate);

	move = rotateMatrix.TransformNomal(move);

	velocity_ = move;

	transform_.translate += move * DeltaTime::GetInstance()->GetGameTime();
}

void Player::BehaviorDashAttackInitialize() {

	attackTimer_ = 0.0f;
	attackTimeMax_ = kDashAttackStart;
	attackPhase_ = 0;
	attackTransform_.Initialize();
	attackTransform_.SetParent(&transform_);
	attackCollider_.SetRadius(2.5f);
	attackCollider_.SetDamage(15.0f);
	attackCollider_.SetDamageCoolTime(0.02f);
	attackCollider_.SetDamageType(0);
	attackCollider_.SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack));
}

void Player::BehaviorDashAttackUpdate() {
	isDash_ = true;
	attackCollider_.SetActive(false);
	attackCollider_.SetTransform(transform_);
	attackCollider_.SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	attackTimer_ += DeltaTime::GetInstance()->GetGameTime();
	switch (attackPhase_) {
	case 0:
		transformModel.rotate.x = Easing(0.0f, Radian(450.0f), attackTimer_, attackTimeMax_, EaseType::kEaseOut);

		if (attackTimer_ >= attackTimeMax_) {
			attackPhase_++;

			Vector3 move = { 0.0f,0.0f,kDashSpeed };
			Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(transform_.rotate);
			velocity_ = rotateMatrix.TransformNomal(move);
			velocity_.y = 0.0f;
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 1.0f, kSoundEffect);

		}
		break;
	default:
		//colliderDimensionType_ = ColliderDimensionType::k2D;
		attackCollider_.SetActive(true);
		Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };
		velocity_ += accelerationVector * DeltaTime::GetInstance()->GetGameTime();
		transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();

		transformModel.rotate.z += Radian(720.0f) * DeltaTime::GetInstance()->GetGameTime();

		if (transform_.translate.y <= 0.0f) {
			transform_.translate.y = 0.0f;
			SoundManager::GetInstance()->SoundPlay("snd_drill", 1.0f, 1.0f, kSoundEffect);
			behaviorRequest_ = Behavior::kDash;
			transformModel.rotate.x = 0.0f;
			transformModel.rotate.z = 0.0f;
		}
		break;
	}


	CollisionManager::GetInstance()->AddColliderList(&attackCollider_);
	attackCollider_.DrawCollider();
}

void Player::BehaviorDashJumpAttackInitialize() {
	InputManager* input = InputManager::GetInstance();
	Vector3 direction = { 0.0f,0.0f,0.0f };
	SoundManager::GetInstance()->SoundPlay("snd_drill", 1.0f, 1.0f, kSoundEffect);
	if (input->IsGamePadConnect()) {
		direction = { input->GetLeftStickDirection().x,0.0f,input->GetLeftStickDirection().y };
	} else {
		if (input->PressKey(DIK_W)) {
			direction.z += 1.0f;
		}
		if (input->PressKey(DIK_S)) {
			direction.z -= 1.0f;
		}
		if (input->PressKey(DIK_D)) {
			direction.x += 1.0f;
		}
		if (input->PressKey(DIK_A)) {
			direction.x -= 1.0f;
		}
	}
	direction = direction.Normalize();
	float direY = std::atan2(direction.x, direction.z);
	direY = Degree(direY);
	//if (GetDownPress()) {
	//velocity_ *= -1.0f;
	transform_.rotate.y += Radian(direY);
	Vector3 move = { 0.0f,0.0f,kDashSpeed };

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(transform_.rotate);

	move = rotateMatrix.TransformNomal(move);

	velocity_ = move;


	transformModel.rotate.x = Radian(90.0f);
	attackTimer_ = 0.0f;
	attackTimeMax_ = kDashAttackStart;
	attackPhase_ = 0;
	attackTransform_.Initialize();
	attackTransform_.SetParent(&transform_);
	attackCollider_.SetRadius(2.5f);
	attackCollider_.SetDamage(15.0f);
	attackCollider_.SetDamageCoolTime(0.02f);
	attackCollider_.SetDamageType(0);
	attackCollider_.SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack));
	velocity_.y = kDashJumpFirstSpeed_;
	useNextAttack_ = false;
}

void Player::BehaviorDashJumpAttackUpdate() {
	isDash_ = true;
	attackCollider_.SetTransform(transform_);
	attackCollider_.SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };
	velocity_ += accelerationVector * DeltaTime::GetInstance()->GetGameTime();
	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();


	if (velocity_.y <= 0.0f) {
		if (useNextAttack_) {
			SoundManager::GetInstance()->SoundPlay("snd_shine", 1.0f, 1.0f, kSoundEffect);
			behaviorRequest_ = Behavior::kDashAttack;
		} else {
			behaviorRequest_ = Behavior::kFall;
		}

		transformModel.rotate.x = 0.0f;
		useNextAttack_ = false;

	} else if (velocity_.y <= 5.0f) {
		transformModel.rotate.x = Easing(0.0f, Radian(90.0f), velocity_.y, 5.0f, EaseType::kConstant);
		transformModel.rotate.z = 0.0f;
		if (!useNextAttack_) {
			if (tutorialUsableDash_) {
				if (GetAttackButtonTrigger()) {
					useNextAttack_ = true;
				}
			}
		}
	} else {
		transformModel.rotate.z += Radian(720.0f) * DeltaTime::GetInstance()->GetGameTime();
	}


	CollisionManager::GetInstance()->AddColliderList(&attackCollider_);
	attackCollider_.DrawCollider();
}

void Player::BehaviorFallInitialize() {
	velocity_.y = 0.0f;
}

void Player::BehaviorFallUpdate() {
	FloatingAccelerationChange();

	Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };
	velocity_ += accelerationVector * DeltaTime::GetInstance()->GetGameTime();
	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();


	if (transform_.translate.y <= kTranslateBlankY) {
		transform_.translate.y = kTranslateBlankY;
		behaviorRequest_ = Behavior::kRoot;
		SoundManager::GetInstance()->SoundPlay("snd_step", 1.0f, 1.0f, kSoundEffect);
	}

	if (tutorialUsableDash_) {
		if (GetAttackButtonTrigger()) {
			SoundManager::GetInstance()->SoundPlay("snd_shine", 1.0f, 1.0f, kSoundEffect);
			behaviorRequest_ = Behavior::kDashAttack;
		}
	}
}

void Player::UpdateFloatingGimmick() {
	return;
	float kFloatingAnimationStep = 2.0f * std::numbers::pi_v<float> / kFloatingAnimationPeriod;
	floatingParameter += kFloatingAnimationStep;

	floatingParameter = std::fmod(floatingParameter, 2.0f * std::numbers::pi_v<float>);

	transformColliderOffset.translate.y = (std::sin(floatingParameter) * kFloatingAmplitude) + kBodyBlankY;
}

void Player::Draw() {
	Transform dashT;
	dashT.Initialize();
	dashT.SetParent(&transformModel);
	dashT.rotate.x = Radian(90.0f);
	if (!isDeath_ || DeltaTime::GetInstance()->GetIsHitStop()) {
		if (behavior_ == Behavior::kDash) {
			Renderer::GetInstance()->DrawShadow(dashT, &model_, { 0.4f,0.4f,1.0f,1.0f });
		} else {
			Renderer::GetInstance()->DrawModel(transformModel, &model_, false);
			Renderer::GetInstance()->DrawShadow(transformModel, &model_, { 0.0f,0.0f,0.0f,1.0f });
		}
	}

	//particles_->Draw();
	hpGauge_->Draw();


	DrawCollider();
}

void Player::RegisterGlobalVariables() {

	const char* groupName = "Player";

	GlobalVariables::GetInstance()->AddValue(groupName, "FloatingAnimationPeriod", kFloatingAnimationPeriod);
	GlobalVariables::GetInstance()->AddValue(groupName, "FloatingAmplitude", kFloatingAmplitude);
}

void Player::ApplyGlobalVariables() {
	const char* groupName = "Player";
	kFloatingAnimationPeriod = GlobalVariables::GetInstance()->GetIntValue(groupName, "FloatingAnimationPeriod");
	kFloatingAmplitude = GlobalVariables::GetInstance()->GetFloatValue(groupName, "FloatingAmplitude");

}

void Player::OnCollision([[maybe_unused]] Collider* other) {
	if (isImmune_ || isDeath_) {
		return;
	}

	//behaviorRequest_ = Behavior::kJump;
	if (other->GetCollisionAttribute() == CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy)) {
		if (isDash_) {
			return;
		}
	}
	Transform damageTransform = transform_;
	damageTransform.translate += Random::GetInstance()->RandomVector3(-Vector3(colliderSize_.x / 2.0f, 0.5f, colliderSize_.z / 2.0f), Vector3(colliderSize_.x / 2.0f, 0.5f, colliderSize_.z / 2.0f));
	damageTransform.translate.y += 1.5f;
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };

	if (damageCoolTimer_ <= 0.0f) {
		currentHP_ -= (other->GetDamage() * DifficultyManager::GetInstance()->GetDamageMagnification());

		DeltaTime::GetInstance()->SetHitStop(other->GetDamage() * 0.01f);
		if (other->GetDamageCoolTime() == -1.0f) {
			damageCoolTimer_ = 3.0f;
		} else {
			damageCoolTimer_ = other->GetDamageCoolTime();
		}
		SoundManager::GetInstance()->SoundPlay("snd_player_damage", 1.0f, 1.0f, kSoundEffect);
		ParticleManager::GetInstance()->SpawnNumbers(other->GetDamage(), damageTransform, { 1.0f,0.5f,0.5f });
		InputManager::GetInstance()->SetVibration(0.1f, 0.1f, 0.5f);

		if (currentHP_ < 0.0f) {
			currentHP_ = 0.0f;
			isDeath_ = true;
			for (uint32_t i = 0; i < 30; i++) {
				ParticleManager::GetInstance()->SpawnParticles("death_cross", transform_.GetWorldPosition());
			}
		}
	}
}
