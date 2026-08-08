#include "Player.h"
#include "GameCamera.h"
void Player::Initialize() {
	transform_.Initialize();
	targetRotateY = 0.0f;
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("drill_ghost"));
	model_.SetBlendMode(BlendMode::kNormalCullNone);
	transform_.translate = { 0.0f, kTranslateBlankY,-30.0f };

	/// HPGauge.
	maxHP_ = 200.0f;
	currentHP_ = maxHP_;

	hpGauge_ = std::make_unique<HPGauge>();
	hpGauge_->Initialize(&currentHP_, maxHP_, { 200.0f,30.0f });
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

	BehaviorAttackInitialize();

	DifficultyManager::GetInstance()->SetPlayerHPData(&currentHP_, maxHP_);

	//emitter_ = std::make_unique<Emitter>();
	//particles_ = std::make_unique<Particles>();
	//particles_->Initialize(TextureManager::GetInstance()->GetTextureInfo("effect_plane"));
	//particles_->SetBillboardType(BillboardType::kAllAxis);
	//emitter_->SetParticle(particles_.get());
	//emitter_->Initialize(transform_, 3, 0.5f);

	//colliderRadius_ = 0.3f;
	colliderSize_ = { 0.5f,1.0f,0.5f };
	colliderType_ = ColliderType::kBox;


	GameCamera::GetInstance()->SetTarget(&transform_);

	LightManager::GetInstance()->CreatePointLight("player_light");
	LightManager::GetInstance()->GetLightData("player_light")->color = { 0.5f,0.5f,1.0f,1.0f };
	LightManager::GetInstance()->GetLightData("player_light")->radius = 5.0f;
	LightManager::GetInstance()->GetLightData("player_light")->intensity = 1.0f;
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
	ImGui::End();
#endif // _DEBUG
	LightManager::GetInstance()->SetLightPos("player_light", transform_.GetWorldPosition());
	LightManager::GetInstance()->SetLightIsActive("player_light", true);


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

	colliderColor_ = { 0.5f,0.5f,1.0f,1.0f };

	if (damageCoolTimer_ > 0.0f) {
		damageCoolTimer_ -= DeltaTime::GetInstance()->GetGameTime();
		if (damageCoolTimer_ <= 0.0f) {
			damageCoolTimer_ = 0.0f;
		}
	}

	transform_.rotate.y = std::fmod(transform_.rotate.y, Radian(360.0f));

	TestWallClamp();

	GameCamera::GetInstance()->SetTargetIsMove(isMoving_);
	GameCamera::GetInstance()->SetTargetIsDash(isDash_);
}

void Player::TestWallClamp() {
	/// ここから地獄
	float distance = transform_.translate.Length();

	float wallDirection = 0.0f;
	if (distance > 70.0f) {
		transform_.translate = transform_.translate.Normalize() * 70.0f;

	}
}

void Player::BehaviorRootInitialize() {
	floatingParameter = 0.0f;

	InitializeFloatingGimmick();
}



void Player::BehaviorRootUpdate() {
	isMoving_ = false;

	velocity_ = { 0.0f,0.0f,0.0f };
	InputManager* input = InputManager::GetInstance();
	if (input->IsGamePadConnect()) {
		velocity_ = { input->GetLeftStickDirection().x,0.0f,input->GetLeftStickDirection().y };

		velocity_ = velocity_.Normalize() * kSpeed;
		if (input->TriggerPadButton(PadButtons::INPUT_R1)) {
			behaviorRequest_ = Behavior::kDash;
		}

		if (GetJumpButtonTrigger()) {
			behaviorRequest_ = Behavior::kJump;
		}

		if (GetAttackButtonTrigger()) {
			behaviorRequest_ = Behavior::kAttack;
		}
	} else {
		if (input->PressKey(DIK_W)) {
			velocity_.z += 1.0f;
		}

		if (input->PressKey(DIK_S)) {
			velocity_.z -= 1.0f;
		}

		if (input->PressKey(DIK_A)) {
			velocity_.x -= 1.0f;
		}

		if (input->PressKey(DIK_D)) {
			velocity_.x += 1.0f;
		}

		if (GetJumpButtonTrigger()) {
			behaviorRequest_ = Behavior::kJump;
		}

		if (input->TriggerKey(DIK_LCONTROL)) {
			behaviorRequest_ = Behavior::kDash;
		}

		if (GetAttackButtonTrigger()) {
			behaviorRequest_ = Behavior::kAttack;
		}

		velocity_ = velocity_.Normalize() * kSpeed;
	}

	if (velocity_.x != 0.0f || velocity_.z != 0.0f) {
		isMoving_ = true;
	}

	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateMatrix(Camera::GetInstance()->GetTransform().rotate);

	velocity_ = cameraRotateMatrix.TransformNomal(velocity_);

	if (isMoving_) {
		targetRotateY = std::atan2(velocity_.x, velocity_.z);
	}

	transform_.rotate.y = LerpShortAngle(transform_.rotate.y, targetRotateY, kCompletionRate);
	//transform_.rotate.y = std::atan2(move.x, move.z);

	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();;

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

void Player::SetNextAttackPhase(float timeMax) {
	attackTimeMax_ = timeMax;
	attackPhase_++;
	attackTimer_ = 0.0f;
}

void Player::AttackFirstInitialize() {
	useNextAttack_ = false;
	attackCollider_.SetRadius(2.5f);
	attackCollider_.SetDamage(35.0f);
	attackCollider_.SetDamageCoolTime(0.1f);
	attackCollider_.SetDamageType(1);
	attackCollider_.SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack));
}

void Player::AttackFirstUpdate() {
	attackCollider_.SetActive(false);
	attackCollider_.SetTransform(transform_);
	attackCollider_.SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
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
	attackCollider_.SetRadius(2.5f);
	attackCollider_.SetDamage(30.0f);
	attackCollider_.SetDamageCoolTime(0.05f);
	attackCollider_.SetDamageType(2);
	attackCollider_.SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack));
}

void Player::AttackSecondUpdate() {
	attackCollider_.SetActive(false);
	attackCollider_.SetTransform(transform_);
	attackCollider_.SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
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
	attackCollider_.SetRadius(2.5f);
	attackCollider_.SetDamage(25.0f);
	attackCollider_.SetDamageCoolTime(0.02f);
	attackCollider_.SetDamageType(3);
	attackCollider_.SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack));
}

void Player::AttackThreeUpdate() {
	attackCollider_.SetActive(false);
	attackCollider_.SetTransform(transform_);
	attackCollider_.SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
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
		if (input->TriggerPadButton(PadButtons::INPUT_R1)) {
			behaviorRequest_ = Behavior::kRoot;
		}

		if (GetJumpButtonTrigger()) {
			behaviorRequest_ = Behavior::kDashJumpAttack;
		}

		transform_.rotate.y += input->GetLeftStickDirection().x * Radian(3.0f);
	} else {
		if (input->TriggerKey(DIK_LCONTROL)) {
			behaviorRequest_ = Behavior::kRoot;
		}

		if (input->PressKey(DIK_A)) {
			transform_.rotate.y -= Radian(3.0f);
		}

		if (input->PressKey(DIK_D)) {
			transform_.rotate.y += Radian(3.0f);
		}

		if (GetJumpButtonTrigger()) {
			behaviorRequest_ = Behavior::kDashJumpAttack;
		}
	}

	Vector3 move = { 0.0f,0.0f,kDashSpeed };

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(transform_.rotate);

	move = rotateMatrix.TransformNomal(move);

	velocity_ = move;

	transform_.translate += move * DeltaTime::GetInstance()->GetGameTime();
}

void Player::BehaviorJumpInitialize() {
	velocity_.y = kJumpFirstSpeed_;


}

void Player::BehaviorJumpUpdate() {

	FloatingAccelerationChange();

	Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };
	velocity_ += accelerationVector * DeltaTime::GetInstance()->GetGameTime();
	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();


	if (transform_.translate.y <= kTranslateBlankY) {
		transform_.translate.y = kTranslateBlankY;
		behaviorRequest_ = Behavior::kRoot;
	}

	if (GetAttackButtonTrigger()) {
		behaviorRequest_ = Behavior::kDashAttack;
	}
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

		}
		break;
	default:
		colliderDimensionType_ = ColliderDimensionType::k2D;
		attackCollider_.SetActive(true);
		Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };
		velocity_ += accelerationVector * DeltaTime::GetInstance()->GetGameTime();
		transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();

		transformModel.rotate.z += Radian(720.0f) * DeltaTime::GetInstance()->GetGameTime();

		if (transform_.translate.y <= 0.0f) {
			transform_.translate.y = 0.0f;
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
	if (GetDownPress()) {
		velocity_ *= -1.0f;	
		transform_.rotate.y -= Radian(180.0f);
	}


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
			if (GetAttackButtonTrigger()) {
				useNextAttack_ = true;
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
	}

	if (GetAttackButtonTrigger()) {
		behaviorRequest_ = Behavior::kDashAttack;
	}
}

void Player::UpdateFloatingGimmick() {
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
	if (behavior_ == Behavior::kDash) {
		Renderer::GetInstance()->DrawShadow(dashT, &model_, { 0.4f,0.4f,1.0f,1.0f });
	} else {
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
	//behaviorRequest_ = Behavior::kJump;
	if (other->GetCollisionAttribute() == CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy)) {
		if (isDash_) {
			return;
		}
	}
	colliderColor_ = { 1.0f,0.0f,0.0f,1.0f };

	if (damageCoolTimer_ <= 0.0f) {
		currentHP_ -= (other->GetDamage() * DifficultyManager::GetInstance()->GetDamageMagnification());

		DeltaTime::GetInstance()->SetHitStop(other->GetDamage() * 0.01f);
		if (other->GetDamageCoolTime() == -1.0f) {
			damageCoolTimer_ = 3.0f;
		} else {
			damageCoolTimer_ = other->GetDamageCoolTime();
		}

		if (currentHP_ < 0.0f) {
			currentHP_ = 0.0f;
		}
	}
}
