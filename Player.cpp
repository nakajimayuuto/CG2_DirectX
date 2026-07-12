#include "Player.h"
#include "GameCamera.h"
void Player::Initialize() {
	transform_.Initialize();
	transform_.translate.y =kBodyBlankY;
	targetRotateY = 0.0f;
	models_["body"].Initialize(ModelManager::GetInstance()->GetModelInfo("player"));

	transformBody_.Initialize();
	transformBody_.translate.y;
	transformBody_.SetParent(&transform_);

	models_["head"].Initialize("player_head");
	transformHead_.Initialize();
	transformHead_.SetParent(&transformBody_);

	models_["LArm"].Initialize("player_left_arm");
	transformLArm_.Initialize();
	transformLArm_.SetParent(&transformBody_);
	transformLArm_.translate.x = -0.5f;

	models_["RArm"].Initialize("player_right_arm");
	transformRArm_.Initialize();
	transformRArm_.SetParent(&transformBody_);
	transformRArm_.translate.x = 0.5f;

	models_["hammer_of_justice"].Initialize("hammer_of_justice");
	transformHammer_.Initialize();
	transformHammer_.SetParent(&transformBody_);
	//transform_.rotate.y = std::atan2(velocity.x, velocity.z);
	//Vector3 velocityXZ = { velocity.x,0.0f,velocity.z };
	//transform_.rotate.x = std::atan2(-velocity.y, velocityXZ.Length());

	behavior_ = Behavior::kRoot;

	InitializeFloatingGimmick();

	collisionAttribute_ = kCollisionAttributePlayer;
	collisionMask_ = kCollisionAttributeEnemy;

	BehaviorAttackInitialize();

	//emitter_ = std::make_unique<Emitter>();
	//particles_ = std::make_unique<Particles>();
	//particles_->Initialize(TextureManager::GetInstance()->GetTextureInfo("effect_plane"));
	//particles_->SetBillboardType(BillboardType::kAllAxis);
	//emitter_->SetParticle(particles_.get());
	//emitter_->Initialize(transform_, 3, 0.5f);

	GameCamera::GetInstance()->SetTarget(&transform_);
}

void Player::InitializeFloatingGimmick() {
	floatingParameter = 0.0f;
}

void Player::Update() {
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
		break;
	case Player::Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	case Player::Behavior::kDash:
		BehaviorDashUpdate();
		break;
	case Player::Behavior::kJump:
		BehaviorJumpUpdate();
		break;
	default:
		break;
	}
	//particles_->Update();


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

	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetDeltaTime();;

	UpdateFloatingGimmick();

}
void Player::BehaviorDashInitialize() {
	workDash_.dashParameter_ = 0.0f;
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

	workDash_.dashParameter_ += DeltaTime::GetInstance()->GetDeltaTime();

	Vector3 move = { 0.0f,0.0f,kDashSpeed };

	transform_.rotate.y;//Camera::GetInstance()->GetTransform().rotate.y;

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(transform_.rotate);
	
	move = rotateMatrix.TransformNomal(move);

	transform_.translate += move * DeltaTime::GetInstance()->GetDeltaTime();
}

void Player::BehaviorJumpInitialize() {
	velocity_.y = kJumpFirstSpeed_;


}

void Player::BehaviorJumpUpdate() {
	transform_.translate += velocity_ * DeltaTime::GetInstance()->GetDeltaTime();

	Vector3 accelerationVector = { 0.0f,-kGravityAcceleration,0.0f };

	velocity_ += accelerationVector;

	if (transform_.translate.y <= kBodyBlankY) {
		transform_.translate.y = kBodyBlankY;
		behaviorRequest_ = Behavior::kRoot;
	}
}

void Player::UpdateFloatingGimmick() {
	float kFloatingAnimationStep = 2.0f * std::numbers::pi_v<float> / kFloatingAnimationPeriod;
	floatingParameter += kFloatingAnimationStep;

	floatingParameter = std::fmod(floatingParameter, 2.0f * std::numbers::pi_v<float>);

	transformBody_.translate.y = (std::sin(floatingParameter) * kFloatingAmplitude);
}

void Player::Draw() {
	Renderer::GetInstance()->DrawModel(transformBody_, &models_["body"], false);
	Renderer::GetInstance()->DrawModel(transformHead_, &models_["head"], false);
	Renderer::GetInstance()->DrawModel(transformLArm_, &models_["LArm"], false);
	Renderer::GetInstance()->DrawModel(transformRArm_, &models_["RArm"], false);

	//models_["RArm"].GetColor();

	Renderer::GetInstance()->DrawShadow(transformBody_, &models_["body"], { 0.0f,0.0f,0.0f,1.0f });
	Renderer::GetInstance()->DrawShadow(transformHead_, &models_["head"], { 0.0f,0.0f,0.0f,1.0f });
	Renderer::GetInstance()->DrawShadow(transformLArm_, &models_["LArm"], { 0.0f,0.0f,0.0f,1.0f });
	Renderer::GetInstance()->DrawShadow(transformRArm_, &models_["RArm"], { 0.0f,0.0f,0.0f,1.0f });

	//particles_->Draw();
}

void Player::RegisterGlobalVariables() {

	const char* groupName = "Player";

	GlobalVariables::GetInstance()->AddValue(groupName, "Head Translate", transformHead_.translate);
	GlobalVariables::GetInstance()->AddValue(groupName, "ArmL Translate", transformLArm_.translate);
	GlobalVariables::GetInstance()->AddValue(groupName, "ArmR Translate", transformRArm_.translate);
	GlobalVariables::GetInstance()->AddValue(groupName, "FloatingAnimationPeriod", kFloatingAnimationPeriod);
	GlobalVariables::GetInstance()->AddValue(groupName, "FloatingAmplitude", kFloatingAmplitude);

	for (uint32_t i = 0; i < kComboNum; i++) {
		GlobalVariables::GetInstance()->AddValue(groupName, std::format("anticipationTime Combo{}", i), kConstAttacks_[i].anticipationTime);
		GlobalVariables::GetInstance()->AddValue(groupName, std::format("anticipationSpeed Combo{}", i), kConstAttacks_[i].anticipationSpeed);
		GlobalVariables::GetInstance()->AddValue(groupName, std::format("chargeTime Combo{}", i), kConstAttacks_[i].chargeTime);
		GlobalVariables::GetInstance()->AddValue(groupName, std::format("chargeSpeed Combo{}", i), kConstAttacks_[i].chargeSpeed);
		GlobalVariables::GetInstance()->AddValue(groupName, std::format("swingTime Combo{}", i), kConstAttacks_[i].swingTime);
		GlobalVariables::GetInstance()->AddValue(groupName, std::format("swingSpeed Combo{}", i), kConstAttacks_[i].swingSpeed);
		GlobalVariables::GetInstance()->AddValue(groupName, std::format("recoveryTime Combo{}", i), kConstAttacks_[i].recoveryTime);
	}
}

void Player::ApplyGlobalVariables() {
	const char* groupName = "Player";
	transformHead_.translate = GlobalVariables::GetInstance()->GetVector3Value(groupName, "Head Translate");
	transformLArm_.translate = GlobalVariables::GetInstance()->GetVector3Value(groupName, "ArmL Translate");
	transformRArm_.translate = GlobalVariables::GetInstance()->GetVector3Value(groupName, "ArmR Translate");
	kFloatingAnimationPeriod = GlobalVariables::GetInstance()->GetIntValue(groupName, "FloatingAnimationPeriod");
	kFloatingAmplitude = GlobalVariables::GetInstance()->GetFloatValue(groupName, "FloatingAmplitude");

	for (uint32_t i = 0; i < kComboNum; i++) {
		kConstAttacks_[i].anticipationTime = GlobalVariables::GetInstance()->GetFloatValue(groupName, std::format("anticipationTime Combo{}", i));
		kConstAttacks_[i].anticipationSpeed = GlobalVariables::GetInstance()->GetFloatValue(groupName, std::format("anticipationSpeed Combo{}", i));
		kConstAttacks_[i].chargeTime = GlobalVariables::GetInstance()->GetFloatValue(groupName, std::format("chargeTime Combo{}", i));
		kConstAttacks_[i].chargeSpeed = GlobalVariables::GetInstance()->GetFloatValue(groupName, std::format("chargeSpeed Combo{}", i));
		kConstAttacks_[i].swingTime = GlobalVariables::GetInstance()->GetFloatValue(groupName, std::format("swingTime Combo{}", i));
		kConstAttacks_[i].swingSpeed = GlobalVariables::GetInstance()->GetFloatValue(groupName, std::format("swingSpeed Combo{}", i));
		kConstAttacks_[i].recoveryTime = GlobalVariables::GetInstance()->GetFloatValue(groupName, std::format("recoveryTime Combo{}", i));
	}
}

void Player::OnCollision([[maybe_unused]] Collider* other) {
	//behaviorRequest_ = Behavior::kJump;
}

float Player::GetSumComboTime(uint32_t index) {
	return kConstAttacks_[index].anticipationTime + kConstAttacks_[index].chargeTime + kConstAttacks_[index].recoveryTime + kConstAttacks_[index].swingTime;
}
