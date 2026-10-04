#include "Player.h"
#define NOMINMAX
#include <algorithm>
#include "GameCamera.h"
#include "ProjectileManager.h"
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
		case Player::Behavior::kShot:
			BehaviorShotInitialize();
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
	case Player::Behavior::kShot:
		BehaviorShotUpdate();
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
		MapChipDigUpdate();

	MovingUpdate();

	MapCollisionUpdate();

	UpdateFloatingGimmick();
}

void Player::BehaviorShotInitialize() {
	floatingParameter = 0.0f;

	InitializeFloatingGimmick();
	DeltaTime::GetInstance()->SetGameTimeSpeed(0.2f);
}

void Player::BehaviorShotUpdate() {
	BlockShotUpdate();

	GravityUpdate();

	MapCollisionUpdate();

	UpdateFloatingGimmick();
}

void Player::BlockShot() {
	ProjectileManager::GetInstance()->CreateBullet(transform_, {0.0f,10.0f,0.0f},BulletType::kBlock,CollisionAttributeName::kCollisionPlayerAttack,10.0f,1.0f);
}

void Player::MapChipDigUpdate() {
	if (InputManager::GetInstance()->TriggerAction(InputAction::ATTACK)) {
	MapChipManager::GetInstance()->DeleteBlock(transform_.translate, 0, -1);
	}
}

void Player::BlockShotUpdate() {
	float attenuation = 1.0f;
	if (onGround_) {
		attenuation = kAttenuation;
	} else {
		attenuation = kJumpAttenuation;
	}

	if (InputManager::GetInstance()->ReleaseAction(InputAction::JUMP)) {
		BlockShot();
		behaviorRequest_ = Behavior::kRoot;
		velocity_.y = kJumpAcceleration;
		DeltaTime::GetInstance()->SetGameTimeSpeed(1.0f);
	}

	// 非入力時は移動減衰をかける.
	if (velocity_.x <= 0.01f && velocity_.x >= -0.01f) {
		velocity_.x = 0.0f;
	} else {
		velocity_.x *= (1.0f - attenuation);
	}
}

void Player::GravityUpdate() {
	if (!onGround_) {
		// 落下速度.
		velocity_ += Vector3(0.0f, -kGravityAcceleration, 0.0f);
		// 速度制限.
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

void Player::MovingUpdate() {
	float attenuation = 1.0f;
	if (onGround_) {
		attenuation = kAttenuation;
	} else {
		attenuation = kJumpAttenuation;
	}

	if (InputManager::GetInstance()->PressAction(InputAction::RIGHT) || InputManager::GetInstance()->PressAction(InputAction::LEFT)) {
		// 左右加速.
		Vector3 acceleration = {};

		if (InputManager::GetInstance()->PressAction(InputAction::RIGHT)) {
			// 左移動中の右入力.
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - attenuation);
			}

			acceleration.x += kAcceleration;

			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;

				turnFirstRotationY_ = transform_.rotate.y;
				turnTimer_ = kTimeTurn;
			}
		} else if (InputManager::GetInstance()->PressAction(InputAction::LEFT)) {
			// 右移動中の左入力.
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - attenuation);
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
			velocity_.x *= (1.0f - attenuation);
		}
	}

	if (InputManager::GetInstance()->TriggerAction(InputAction::JUMP)) {
		behaviorRequest_ = Behavior::kShot;
	}


	GravityUpdate();
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

void Player::CheckTutorialFlag() {
	tutorialUsableMove_ = true;
	tutorialUsableJump_ = true;
	tutorialUsableDash_ = true;
	tutorialUsableAttack_ = true;

}

void Player::CheckTutorialUpdate() {
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
		Renderer::GetInstance()->DrawModel(transformModel, &model_, false);
		Renderer::GetInstance()->DrawShadow(transformModel, &model_, { 0.0f,0.0f,0.0f,1.0f });
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
