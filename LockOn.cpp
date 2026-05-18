#include "LockOn.h"
#include "Player.h"

void LockOn::Initialize() {
	sprite_.Initialize(TextureManager::GetInstance()->GetTextureInfo("reticle"));
	sprite_.SetColor({ 1.0f,0.0f,0.0f,1.0f });
	transform_.Initialize();
}

void LockOn::Update(Player* player, std::list<BaseEnemy*>& enemies) {
	std::list<std::pair<float, BaseEnemy*>>targets;

	Vector3 playerPos = player->GetWorldPosition();
	playerPos = Camera::GetInstance()->GetCameraVector3(playerPos, Matrix4x4::Identity());


	for (BaseEnemy* enemy : enemies) {
		Vector3 positionWorld = enemy->GetWorldPosition();

		Vector3 positionScreen = Camera::GetInstance()->GetCameraVector3(positionWorld, Matrix4x4::Identity());

		if (positionScreen.z <= playerPos.z) {
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
	//sprite_.SetIsVisible(false);

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
}

void LockOn::Draw() {
	sprite_.Draw(transform_);
}
