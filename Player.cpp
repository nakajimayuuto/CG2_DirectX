#include "Player.h"

void Player::Initialize() {
	transform_.Initialize();
	targetRotateY = 0.0f;
	models_["body"].Initialize(ModelManager::GetInstance()->GetModelInfo("player"));

	transformBody_.Initialize();
	transformBody_.translate.y = kBodyBlankY;
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

	BehaviorAttackInitialize();
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

		if (input->TriggerPadButton(PadButtons::INPUT_L1)) {
			behaviorRequest_ = Behavior::kAttack;
		}

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

		if (input->TriggerMouse(MouseButtons::MOUSE_LEFT)) {
			behaviorRequest_ = Behavior::kAttack;
		}

		if (input->TriggerMouse(MouseButtons::MOUSE_RIGHT)) {
			behaviorRequest_ = Behavior::kDash;
		}

		if (input->TriggerKey(DIK_SPACE)) {
			behaviorRequest_ = Behavior::kJump;
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

	transform_.translate += velocity_;

	UpdateFloatingGimmick();

}


void Player::BehaviorAttackInitialize() {
	hammerAnimationTimer_ = 0.0f;

	transformHammer_.rotate.x = 0.0f;

	attackPhase_ = kCharge;
}

void Player::BehaviorAttackUpdate() {
	hammerAnimationTimer_ += DeltaTime::GetInstance()->GetDeltaTime();

	transformHammer_.rotate.x = Easing(kStartHammerRotateX, kStampHammerRotateX, hammerAnimationTimer_, kStampAnimationMaxTime, EaseType::kEaseInBack);

	if (hammerAnimationTimer_ > kStampAnimationMaxTime) {
		behaviorRequest_ = Behavior::kRoot;
	}
}

void Player::BehaviorDashInitialize() {
	workDash_.dashParameter_ = 0.0f;
	transform_.rotate.y = targetRotateY;
}

void Player::BehaviorDashUpdate() {
	workDash_.dashParameter_ += DeltaTime::GetInstance()->GetDeltaTime();

	Vector3 move = { 0.0f,0.0f,1.0f };

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix(transform_.rotate);

	move = rotateMatrix.TransformNomal(move);

	transform_.translate += move;

	if (workDash_.dashParameter_ >= kBehaviorDashTime) {
		behaviorRequest_ = Behavior::kRoot;
	}
}

void Player::BehaviorJumpInitialize(){
	transformBody_.translate.y = kBodyBlankY;
	velocity_.y = kJumpFirstSpeed_;


}

void Player::BehaviorJumpUpdate(){
	transform_.translate += velocity_;

	Vector3 accelerationVector = {0.0f,-kGravityAcceleration,0.0f};

	velocity_ += accelerationVector;

	if (transform_.translate.y <= 0.0f) {
		transform_.translate.y = 0.0f;
		behaviorRequest_ = Behavior::kRoot;
	}
}

void Player::UpdateFloatingGimmick() {
	float kFloatingAnimationStep = 2.0f * std::numbers::pi_v<float> / kFloatingAnimationPeriod;
	floatingParameter += kFloatingAnimationStep;

	floatingParameter = std::fmod(floatingParameter, 2.0f * std::numbers::pi_v<float>);

	transformBody_.translate.y = (std::sin(floatingParameter) * kFloatingAmplitude) + kBodyBlankY;
}

void Player::Draw() {
	Renderer::GetInstance()->DrawModel(transformBody_, &models_["body"]);
	Renderer::GetInstance()->DrawModel(transformHead_, &models_["head"]);
	Renderer::GetInstance()->DrawModel(transformLArm_, &models_["LArm"]);
	Renderer::GetInstance()->DrawModel(transformRArm_, &models_["RArm"]);

	if (behavior_ == Behavior::kAttack) {
		Renderer::GetInstance()->DrawModel(transformHammer_, &models_["hammer_of_justice"]);
	}
}

void Player::RegisterGlobalVariables() {

	const char* groupName = "Player";

	GlobalVariables::GetInstance()->AddValue(groupName, "Head Translate", transformHead_.translate);
	GlobalVariables::GetInstance()->AddValue(groupName, "ArmL Translate", transformLArm_.translate);
	GlobalVariables::GetInstance()->AddValue(groupName, "ArmR Translate", transformRArm_.translate);
	GlobalVariables::GetInstance()->AddValue(groupName, "FloatingAnimationPeriod", kFloatingAnimationPeriod);
	GlobalVariables::GetInstance()->AddValue(groupName, "FloatingAmplitude", kFloatingAmplitude);
}

void Player::ApplyGlobalVariables() {
	const char* groupName = "Player";
	transformHead_.translate = GlobalVariables::GetInstance()->GetVector3Value(groupName, "Head Translate");
	transformLArm_.translate = GlobalVariables::GetInstance()->GetVector3Value(groupName, "ArmL Translate");
	transformRArm_.translate = GlobalVariables::GetInstance()->GetVector3Value(groupName, "ArmR Translate");
	kFloatingAnimationPeriod = GlobalVariables::GetInstance()->GetIntValue(groupName, "FloatingAnimationPeriod");
	kFloatingAmplitude = GlobalVariables::GetInstance()->GetFloatValue(groupName, "FloatingAmplitude");
}