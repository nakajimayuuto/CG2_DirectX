#include "Player.h"

void Player::Initialize() {
	transform_.Initialize();
	transform_.translate.y = 1.5f;
	targetRotateY = 0.0f;
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));

	transformBody_.Initialize();
	transformBody_.SetParent(&transform_);

	modelHead_.Initialize("player_head");
	transformHead_.Initialize();
	transformHead_.SetParent(&transformBody_);

	modelLArm_.Initialize("player_left_arm");
	transformLArm_.Initialize();
	transformLArm_.SetParent(&transformBody_);
	transformLArm_.translate.x = -0.5f;

	modelRArm_.Initialize("player_right_arm");
	transformRArm_.Initialize();
	transformRArm_.SetParent(&transformBody_);
	transformRArm_.translate.x = 0.5f;
	//transform_.rotate.y = std::atan2(velocity.x, velocity.z);
	//Vector3 velocityXZ = { velocity.x,0.0f,velocity.z };
	//transform_.rotate.x = std::atan2(-velocity.y, velocityXZ.Length());

	InitializeFloatingGimmick();
}

void Player::InitializeFloatingGimmick() {
	floatingParameter = 0.0f;
}

void Player::Update() {
	isMoving_ = false;

	Vector3 move = { 0.0f,0.0f,0.0f };
	InputManager* input = InputManager::GetInstance();
	if (input->IsGamePadConnect()) {
		move = {input->GetLeftStickDirection().x,0.0f,input->GetLeftStickDirection().y};

		move = move.Normalize() * kSpeed;
	} else {
		if (input->PressKey(DIK_W)) {
			move.z += 1.0f;
		}

		if (input->PressKey(DIK_S)) {
			move.z -= 1.0f;
		}
		
		if (input->PressKey(DIK_A)) {
			move.x -= 1.0f;
		}
		
		if (input->PressKey(DIK_D)) {
			move.x += 1.0f;
		}

		move = move.Normalize() * kSpeed;
	}

	if (move.x != 0.0f || move.z != 0.0f) {
		isMoving_ = true;
	}

	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateMatrix(Camera::GetInstance()->GetTransform().rotate);

	move = cameraRotateMatrix.TransformNomal(move);

	if (isMoving_) {
		targetRotateY = std::atan2(move.x, move.z);
	}
	
	transform_.rotate.y = LerpShortAngle(transform_.rotate.y,targetRotateY,kCompletionRate);
	//transform_.rotate.y = std::atan2(move.x, move.z);

	transform_.translate += move;

	UpdateFloatingGimmick();
}

void Player::UpdateFloatingGimmick(){
	floatingParameter += kFloatingAnimationStep;

	floatingParameter = std::fmod(floatingParameter,2.0f * std::numbers::pi_v<float>);

	transformBody_.translate.y = std::sin(floatingParameter) * kFloatingAmplitude;
}

void Player::Draw() {
	Renderer::GetInstance()->DrawModel(transformBody_,&model_);
	Renderer::GetInstance()->DrawModel(transformHead_,&modelHead_);
	Renderer::GetInstance()->DrawModel(transformLArm_,&modelLArm_);
	Renderer::GetInstance()->DrawModel(transformRArm_,&modelRArm_);
}

void Player::RegisterGlobalVariables() {

	const char* groupName = "Player";

	GlobalVariables::GetInstance()->AddValue(groupName, "Head Translate",transformHead_.translate);
	GlobalVariables::GetInstance()->AddValue(groupName, "ArmL Translate",transformLArm_.translate);
	GlobalVariables::GetInstance()->AddValue(groupName, "ArmR Translate",transformRArm_.translate);
}

void Player::ApplyGlobalVariables() {
	const char* groupName = "Player";

}