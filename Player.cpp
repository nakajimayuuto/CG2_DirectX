#include "Player.h"
#include "NormalBullet.h"
#include "HomingBullet.h"
#include "./Scene/GameScene.h"
#include "RailCameraController.h"
#include "LockOn.h"

Player::~Player() {
	delete cameraController_;
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
	Reticle2DUpdate();
	AttackUpdate();

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
	if (InputManager::GetInstance()->IsGamePadConnect()) {
		Vector3 positionReticle = transform3DReticle_.GetAffineMatrix().GetMatrixToTranslate();

		transform2DReticle_.translate = Camera::GetInstance()->GetCameraVector3(positionReticle, Matrix4x4::Identity());
	} else {
		if (cameraController_->GetCameraType() == RailCameraController::CameraType::kFirstPoint) {
			transform2DReticle_.translate = {
				static_cast<float>(Environment::GetInstance()->GetWindowSize().width) / 2.0f,
				static_cast<float>(Environment::GetInstance()->GetWindowSize().height) / 2.0f,
				kDistancePlayerTo3DReticle_
			};
		} else {
			Vector2 mousePos = InputManager::GetInstance()->GetMousePos();
			transform2DReticle_.translate = { mousePos.x,mousePos.y,0.0f };

			Matrix4x4 matVPV = Camera::GetInstance()->GetVPVMatrix(transform_.GetAffineMatrix());
			matVPV = matVPV.Inverse();
			Vector3 posNear = { mousePos.x,mousePos.y,0.0f };
			Vector3 posFar = { mousePos.x,mousePos.y,1.0f };

			posNear = matVPV.MatrixTransform(posNear);
			posFar = matVPV.MatrixTransform(posFar);

			Vector3 mouseDirection = posFar - posNear;
			mouseDirection.z *= 0.5f;
			mouseDirection = mouseDirection.Normalize();
			transform3DReticle_.Initialize();

			transform3DReticle_.translate = (mouseDirection);
		}
	}

}

void Player::MoveUpdate() {
	InputManager* input = InputManager::GetInstance();
	Vector3 move = { 0.0f,0.0f,0.0f };

	if (input->IsGamePadConnect()) {
		move.x += input->GetLeftStickDirection().x * kCharacterSpeed;
		move.y += input->GetLeftStickDirection().y * kCharacterSpeed;
	} else {
		if (input->PressKey(DIK_LEFT) || input->PressKey(DIK_A)) {
			move.x -= kCharacterSpeed;
		} else if (input->PressKey(DIK_RIGHT) || input->PressKey(DIK_D)) {
			move.x += kCharacterSpeed;
		}

		if (input->PressKey(DIK_UP) || input->PressKey(DIK_W)) {
			move.y += kCharacterSpeed;
		} else if (input->PressKey(DIK_DOWN) || input->PressKey(DIK_S)) {
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
		if (cameraController_->GetCameraType() == RailCameraController::CameraType::kFirstPoint) {
			Vector3 cameraRotate;
			if (input->GetInstance()->GetMouse().GetMoveLength() >= 0.2f) {
				cameraRotate.x = input->GetMouse().GetMove().y;
				cameraRotate.y = input->GetMouse().GetMove().x;
				cameraRotate.z = 0.0f;
			} else {
				cameraRotate = { 0.0f,0.0f,0.0f };
			}

			input->GetMouse().SetCursorPosition(
				{
					static_cast<float>(Environment::GetInstance()->GetWindowSize().width) / 2.0f,
					static_cast<float>(Environment::GetInstance()->GetWindowSize().height) / 2.0f
				}
			);

			transform_.rotate += (cameraRotate.Normalize() * kFirstPointRotSpeed);
		}
	}

	Vector2 rotate;

	if (cameraController_->GetCameraType() == RailCameraController::CameraType::kFirstPoint) {
		rotate.x = kFirstPointRotateLimitX;
		rotate.y = kFirstPointRotateLimitY;
	} else {
		rotate.x = kRotateLimitX;
		rotate.y = kRotateLimitY;
	}

	transform_.rotate.y = std::max(transform_.rotate.y, -rotate.y);
	transform_.rotate.y = std::min(transform_.rotate.y, rotate.y);

	transform_.rotate.x = std::max(transform_.rotate.x, -rotate.x);
	transform_.rotate.x = std::min(transform_.rotate.x, rotate.x);
}

void Player::AttackUpdate() {
	if (InputManager::GetInstance()->TriggerKey(DIK_SPACE) || InputManager::GetInstance()->TriggerPadButton(INPUT_R2)) {
		Vector3 velocity(0.0f, 0.0f, kBulletSpeed);
		BaseBullet* newBullet = new NormalBullet();

		if (cameraController_->GetCameraType() == RailCameraController::CameraType::kFirstPoint) {
			velocity = transform3DReticle_.GetAffineMatrix().GetMatrixToTranslate() - transform_.GetAffineMatrix().GetMatrixToTranslate();
			velocity.x *= -1.0f;
			velocity.y *= -1.0f;
			velocity = velocity.Normalize() * kBulletSpeed;
		} else {
			//velocity = transform_.GetAffineMatrix().TransformNomal(velocity);
			if (isLockOn_) {
				if (target_.expired()) {
					return;
				}

				std::weak_ptr<Collider> target = target_;

				velocity = target.lock()->GetWorldPosition() - GetWorldPosition();
				velocity = velocity.Normalize() * kBulletSpeed;

				delete newBullet;
				newBullet = new HomingBullet();
				dynamic_cast<HomingBullet*>(newBullet)->SetTarget(target);
			} else {
				velocity = transform3DReticle_.GetAffineMatrix().GetMatrixToTranslate() - transform_.GetAffineMatrix().GetMatrixToTranslate();
				velocity = velocity.Normalize() * kBulletSpeed;
			}
		}



		newBullet->Initialize("player_bullet", transform_.GetAffineMatrix().GetMatrixToTranslate(), velocity);
		newBullet->SetCollisionAttribute(kCollisionAttributePlayer);
		newBullet->SetCollisionMask(kCollisionAttributeEnemy);
		dynamic_cast<GameScene*>(gameScene_)->AddBullet(newBullet);
	}

}

void Player::Draw() {

	if (cameraController_->GetCameraType() == RailCameraController::CameraType::kThirdPoint) {
		model_.Draw(transform_);
	} else {
		sprite_.Draw(transform2DReticle_);
	}
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
