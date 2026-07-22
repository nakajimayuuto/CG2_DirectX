#include "Player.h"
#include "GameCamera.h"
void Player::Initialize() {
	transform_.Initialize();
	targetRotateY = 0.0f;
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("drill_ghost"));
	model_.SetBlendMode(BlendMode::kNormalCullNone);
	transform_.translate = {0.0f, kTranslateBlankY,-30.0f };

	/// HPGauge.
	maxHP_ = 200.0f;
	currentHP_ = maxHP_;

	hpGauge_ = std::make_unique<HPGauge>();
	hpGauge_->Initialize(&currentHP_, maxHP_, {200.0f,30.0f});
	hpGauge_->SetPosition({-500.0f,300.0f});

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

	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer);
	collisionMask_ = (
		CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy) |
		CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack)
		);

	BehaviorAttackInitialize();

	DifficultyManager::GetInstance()->SetPlayerHPData(&currentHP_,maxHP_);

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
}

void Player::InitializeFloatingGimmick() {
	floatingParameter = 0.0f;
}

void Player::Update() {
#ifdef _DEBUG
	ImGui::Begin("player");
	ImGui::DragFloat("HP", &currentHP_, 1.0f, 0.0f, maxHP_);
	ImGui::End();
#endif // _DEBUG


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
		default:
			break;
		}

		behaviorRequest_ = std::nullopt;
	}

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
		break;
	case Player::Behavior::kJump:
		BehaviorJumpUpdate();

		CollisionManager::GetInstance()->AddColliderList(this);
		break;
	default:
		break;
	}
	//particles_->Update();

	colliderColor_ = { 0.5f,0.5f,1.0f,1.0f };

	if (damageCoolTimer_ > 0.0f) {
		damageCoolTimer_ -= DeltaTime::GetInstance()->GetGameTime();
		if (damageCoolTimer_ <= 0.0f) {
			damageCoolTimer_ = 0.0f;
		}
	}

	transform_.rotate.y = std::fmod(transform_.rotate.y, Radian(360.0f));

	GameCamera::GetInstance()->SetTargetIsMove(isMoving_);
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

		if (input->TriggerPadButton(PadButtons::INPUT_A) || input->TriggerPadButton(PadButtons::INPUT_B)) {
			behaviorRequest_ = Behavior::kJump;
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

		if (input->TriggerKey(DIK_SPACE)) {
			behaviorRequest_ = Behavior::kJump;
		}

		if (input->TriggerKey(DIK_LCONTROL)) {
			behaviorRequest_ = Behavior::kDash;
		}

		if (input->TriggerMouse(MouseButtons::MOUSE_LEFT)) {
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
void Player::BehaviorAttackInitialize(){
	attackTimer_ = 0.0f;
	attackTimeMax_ = kAttackFirstStart;
	attackComboPhase_ = 0;
	attackPhase_ = 0;
	attackTransform_.Initialize();
	attackTransform_.SetParent(&transformColliderOffset);
	transformModel.SetParent(&attackTransform_); 
	AttackFirstInitialize();
}
void Player::BehaviorAttackUpdate(){
	attackTimer_ += DeltaTime::GetInstance()->GetGameTime();


	switch (attackComboPhase_){
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
void Player::BehaviorAttackFinished(){
	behaviorRequest_ = Behavior::kRoot;
	transformModel.SetParent(&transformColliderOffset);
}

void Player::SetNextAttackPhase(float timeMax) {
	attackTimeMax_ = timeMax;
	attackPhase_++;
	attackTimer_ = 0.0f;
}

void Player::AttackFirstInitialize(){
	attackCollider_.SetRadius(1.0f);
	attackCollider_.SetDamage(35.0f);
	attackCollider_.SetDamageCoolTime(0.1f);
	attackCollider_.SetDamageType(1);
	attackCollider_.SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack));
}

void Player::AttackFirstUpdate(){
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
		attackTransform_.rotate.x = Easing(kAttackFirstStartModelRotateX,kAttackFirstSpinModelRotateX, attackTimer_, attackTimeMax_, EaseType::kConstant);

		if (attackTimer_ >= attackTimeMax_) {
			SetNextAttackPhase(kAttackFirstFinish);
		}
		break;
	case 2:
		attackTransform_.rotate.x = Easing(kAttackFirstSpinModelRotateX, Radian(360.0f), attackTimer_, attackTimeMax_, EaseType::kEaseOut);
		transformModel.translate.y = Easing( kAttackFirstSpinModelPosY, 0.0f, attackTimer_, attackTimeMax_, EaseType::kEaseOut);
		
		if (attackTimer_ >= attackTimeMax_) {
			BehaviorAttackFinished();
		}
		break;
	}

	CollisionManager::GetInstance()->AddColliderList(&attackCollider_);
	attackCollider_.DrawCollider();
}

void Player::AttackSecondInitialize(){
}

void Player::AttackSecondUpdate(){
}

void Player::AttackThreeInitialize(){
}

void Player::AttackThreeUpdate(){
}

void Player::BehaviorDashInitialize() {

	transform_.rotate.y = targetRotateY;
}

void Player::BehaviorDashUpdate() {
	isMoving_ = true;
	InputManager* input = InputManager::GetInstance();
	if (input->IsGamePadConnect()) {
		if (input->TriggerPadButton(PadButtons::INPUT_R1)) {
			behaviorRequest_ = Behavior::kRoot;
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
	}

	Vector3 move = { 0.0f,0.0f,kDashSpeed };

	transform_.rotate.y;//Camera::GetInstance()->GetTransform().rotate.y;

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(transform_.rotate);

	move = rotateMatrix.TransformNomal(move);

	transform_.translate += move * DeltaTime::GetInstance()->GetGameTime();
}

void Player::BehaviorJumpInitialize() {
	velocity_.y = kJumpFirstSpeed_;


}

void Player::BehaviorJumpUpdate() {
	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetGameTime();

	Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };

	velocity_ += accelerationVector;

	if (transform_.translate.y <= kTranslateBlankY) {
		transform_.translate.y = kTranslateBlankY;
		behaviorRequest_ = Behavior::kRoot;
	}
}

void Player::UpdateFloatingGimmick() {
	float kFloatingAnimationStep = 2.0f * std::numbers::pi_v<float> / kFloatingAnimationPeriod;
	floatingParameter += kFloatingAnimationStep;
	
	floatingParameter = std::fmod(floatingParameter, 2.0f * std::numbers::pi_v<float>);
	
	transformColliderOffset.translate.y = (std::sin(floatingParameter) * kFloatingAmplitude) + kBodyBlankY;
}

void Player::Draw() {
	Renderer::GetInstance()->DrawModel(transformModel, &model_, false);

	Renderer::GetInstance()->DrawShadow(transformModel, &model_, { 0.0f,0.0f,0.0f,1.0f });

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