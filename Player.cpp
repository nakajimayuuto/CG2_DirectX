#include "Player.h"
#include "PlayerBullet.h"
#include "./Scene/GameScene.h"

Player::~Player() {
}

void Player::Initialize(const Vector3& position) {
	transform_.Initialize();
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
	model2_.Initialize(ModelManager::GetInstance()->GetModelInfo("enemy_bullet"));
	sprite_.Initialize(TextureManager::GetInstance()->GetTextureInfo("reticle"));
	sprite_.SetColor({ 1.0f,0.0f,0.0f,1.0f });

	transform_.translate = position;

	transform3DReticle_.Initialize();
	transform3DReticle_.translate = { 0.0f,0.0f,50.0f };
	transform3DReticle_.SetParent(&transform_);
	transform2DReticle_.Initialize();

	SetCollisionAttribute(kCollisionAttributePlayer);
	SetCollisionMask(kCollisionAttributeEnemy);
}

void Player::Update() {
	MoveUpdate();
	RotateUpdate();
	AttackUpdate();
	Reticle2DUpdate();

	ImGui::Begin("Player");
	ImGui::DragFloat3("transform", reinterpret_cast<float*>(&transform_.translate.x), 0.1f, -300.0f, 300.0f);
	ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&transform_.rotate.x), 0.01f, -6.0f, 6.0f);
	ImGui::DragFloat3("transform3D", reinterpret_cast<float*>(&transform3DReticle_.translate.x), 0.1f, -300.0f, 300.0f);
	ImGui::DragFloat3("transform2D", reinterpret_cast<float*>(&transform2DReticle_.translate.x), 0.1f, -300.0f, 300.0f);
	ImGui::End();
}

void Player::OnCollision() {

}

void Player::Reticle2DUpdate() {
	Vector3 positionReticle = transform3DReticle_.GetAffineMatrix().GetMatrixToTranslate();

	transform2DReticle_.translate = Camera::GetInstance()->GetCameraVector3(positionReticle, Matrix4x4::Identity());
}

void Player::MoveUpdate() {
	InputManager* input = InputManager::GetInstance();
	Vector3 move = { 0.0f,0.0f,0.0f };

	if (input->IsGamePadConnect()) {
			move.x += input->GetLeftStickDirection().x * kCharacterSpeed;
			move.y += input->GetLeftStickDirection().y * kCharacterSpeed;
	} else {
		if (input->PressKey(DIK_LEFT)) {
			move.x -= kCharacterSpeed;
		} else if (input->PressKey(DIK_RIGHT)) {
			move.x += kCharacterSpeed;
		}

		if (input->PressKey(DIK_UP)) {
			move.y += kCharacterSpeed;
		} else if (input->PressKey(DIK_DOWN)) {
			move.y -= kCharacterSpeed;
		}
	}

	transform_.translate += move;

	transform_.translate.x = std::max(transform_.translate.x, -kMoveLimitX);
	transform_.translate.x = std::min(transform_.translate.x, kMoveLimitX);
	transform_.translate.y = std::max(transform_.translate.y, -kMoveLimitY);
	transform_.translate.y = std::min(transform_.translate.y, kMoveLimitY);
}

void Player::RotateUpdate() {
	InputManager* input = InputManager::GetInstance();
	if (input->IsGamePadConnect()) {
		transform_.rotate.y += input->GetRightStickDirection().x * kRotSpeed;
		transform_.rotate.x -= input->GetRightStickDirection().y * kRotSpeed;
	} else {
		if (input->PressKey(DIK_A)) {
			transform_.rotate.y -= kRotSpeed;
		} else if (input->PressKey(DIK_D)) {
			transform_.rotate.y += kRotSpeed;
		}

		if (input->PressKey(DIK_W)) {
			transform_.rotate.x -= kRotSpeed;
		} else if (input->PressKey(DIK_S)) {
			transform_.rotate.x += kRotSpeed;
		}
	}

	transform_.rotate.y = std::max(transform_.rotate.y, -kRotateLimitY);
	transform_.rotate.y = std::min(transform_.rotate.y, kRotateLimitY);

	transform_.rotate.x = std::max(transform_.rotate.x, -kRotateLimitX);
	transform_.rotate.x = std::min(transform_.rotate.x, kRotateLimitX);
}

void Player::AttackUpdate() {
	if (InputManager::GetInstance()->TriggerKey(DIK_SPACE)) {
		Vector3 velocity(0.0f, 0.0f, kBulletSpeed);

		//velocity = transform_.GetAffineMatrix().TransformNomal(velocity);
		velocity = transform3DReticle_.GetAffineMatrix().GetMatrixToTranslate() - transform_.GetAffineMatrix().GetMatrixToTranslate();
		velocity = velocity.Normalize() * kBulletSpeed;

		BaseBullet* newBullet = new PlayerBullet;
		newBullet->Initialize("player_bullet", transform_.GetAffineMatrix().GetMatrixToTranslate(), velocity);

		dynamic_cast<GameScene*>(gameScene_)->AddBullet(newBullet);
	}

}

void Player::Draw() {
	sprite_.Draw(transform2DReticle_);

	model_.Draw(transform_);

}

void Player::RegisterGlobalVariables() {
	const std::string name = "player";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	globalVariables->AddValue(name, "CharacterSpeed", kCharacterSpeed);
	globalVariables->AddValue(name, "RotSpeed", kRotSpeed);
	globalVariables->AddValue(name, "MoveLimitX", kMoveLimitX);
	globalVariables->AddValue(name, "MoveLimitY", kMoveLimitY);
	globalVariables->AddValue(name, "RotateLimitY", kRotateLimitY);
	globalVariables->AddValue(name, "BulletSpeed", kBulletSpeed);
}

void Player::ApplyGlobalVariables() {
	const std::string name = "player";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	kCharacterSpeed = globalVariables->GetFloatValue(name, "CharacterSpeed");
	kRotSpeed = globalVariables->GetFloatValue(name, "RotSpeed");
	kMoveLimitX = globalVariables->GetFloatValue(name, "MoveLimitX");
	kMoveLimitY = globalVariables->GetFloatValue(name, "MoveLimitY");
	kRotateLimitY = globalVariables->GetFloatValue(name, "RotateLimitY");
	kBulletSpeed = globalVariables->GetFloatValue(name, "BulletSpeed");

}
