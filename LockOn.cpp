#include "LockOn.h"
#include "Player.h"

void LockOn::Initialize() {
	sprite_.Initialize(TextureManager::GetInstance()->GetTextureInfo("reticle"));
	sprite_.SetColor({ 1.0f,0.0f,0.0f,1.0f });
	transform_.Initialize();
}

void LockOn::Update(std::list<std::unique_ptr<Enemy>>& enemies) {


	if (isLockOn_) {

	} else {
		if (InputManager::GetInstance()->TriggerPadButton(PadButtons::INPUT_Y) || InputManager::GetInstance()->TriggerKey(DIK_LSHIFT)) {
			TargetLockOn(enemies);
		}
	}


	/*
	std::list<std::pair<float, Enemy*>>targets;

	Vector3 playerPos = player->GetWorldPosition();
	playerPos = Camera::GetInstance()->GetCameraVector3(playerPos, Matrix4x4::Identity());


	for (Enemy* enemy : enemies) {
		Vector3 positionWorld = enemy->GetWorldPosition();

		Vector3 positionScreen = Camera::GetInstance()->GetCameraVector3(positionWorld, Matrix4x4::Identity());

		if (positionWorld.z <= playerPos.z) {
			continue;
		}

		Vector2 positionScreenV2 = { positionScreen.x,positionScreen.y };

		float distance = (player->GetPositionReticle2D() - positionScreenV2).Length();

		if (distance <= kDistanceLockOn) {
			targets.emplace_back(std::make_pair(distance, enemy));
		}
	}

	target_ = nullptr;
	isLockOn_ = false;
	sprite_.SetIsVisible(false);

	if (!targets.empty()) {
		targets.sort();

		target_ = targets.front().second;

		transform_.translate = target_->GetWorldPosition();

		transform_.translate = Camera::GetInstance()->GetCameraVector3(transform_.translate, Matrix4x4::Identity());
		sprite_.SetIsVisible(true);
		isLockOn_ = true;
	} else {
		transform_.translate.x = player->GetPositionReticle2D().x;
		transform_.translate.y = player->GetPositionReticle2D().y;
	}
	*/
}

void LockOn::Draw() {
	sprite_.Draw(transform_);
}

void LockOn::TargetLockOn(std::list<std::unique_ptr<Enemy>>& enemies) {
	std::list<std::pair<float, Enemy*>>targets;

	//Vector3 playerPos = player->GetWorldPosition();
	//playerPos = Camera::GetInstance()->GetCameraVector3(playerPos, Matrix4x4::Identity());


	for (const std::unique_ptr<Enemy>& enemy : enemies) {
		Vector3 positionWorld = enemy->GetWorldPosition();

		Vector3 positionScreen = Camera::GetInstance()->GetCameraVector3(positionWorld, Matrix4x4::Identity());

		if (minDistance_ <= positionWorld.z && positionWorld.z <= maxDistance_) {

			float arcTangent = std::atan2(std::sqrt(positionScreen.x * positionScreen.x + positionScreen.y * positionScreen.y),positionScreen.z);
			
			if (std::fabs(arcTangent) <= angleRange_) {
				targets.emplace_back(std::make_pair(arcTangent, enemy.get()));
			}
		}
	}

	target_ = nullptr;
	isLockOn_ = false;
	sprite_.SetIsVisible(false);

	if (!targets.empty()) {
		targets.sort([](auto& pair1, auto& pair2) {return pair1.first < pair2.first; });

		target_ = targets.front().second;

	//	transform_.translate = target_->GetWorldPosition();
	//
	//	transform_.translate = Camera::GetInstance()->GetCameraVector3(transform_.translate, Matrix4x4::Identity());
	//	sprite_.SetIsVisible(true);
	//	isLockOn_ = true;
	//} else {
	//	transform_.translate.x = player->GetPositionReticle2D().x;
	//	transform_.translate.y = player->GetPositionReticle2D().y;
	}
}
