#include "LockOn.h"
#include "Player.h"

void LockOn::Initialize() {
	sprite_.Initialize(TextureManager::GetInstance()->GetTextureInfo("reticle"));
	sprite_.SetColor({ 1.0f,0.0f,0.0f,1.0f });
	transform_.Initialize();
}

void LockOn::Update(std::weak_ptr<Player> player, std::list<std::weak_ptr<BaseEnemy>>& enemies) {
	std::list<std::pair<float, std::weak_ptr<BaseEnemy>>>targets;

	Vector3 playerPos = player.lock().get()->GetWorldPosition();
	playerPos = Camera::GetInstance()->GetCameraVector3(playerPos, Matrix4x4::Identity());


	for (std::weak_ptr<BaseEnemy> enemy : enemies) {
		Vector3 positionWorld = enemy.lock().get()->GetWorldPosition();

		Vector3 positionScreen = Camera::GetInstance()->GetCameraVector3(positionWorld, Matrix4x4::Identity());

		if (positionScreen.z <= playerPos.z) {
			continue;
		}

		Vector2 positionScreenV2 = { positionScreen.x,positionScreen.y };

		float distance = (player.lock().get()->GetPositionReticle2D() - positionScreenV2).Length();

		if (distance <= kDistanceLockOn) {
			targets.emplace_back(std::make_pair(distance, enemy));
		}
	}

	isLockOn_ = false;
	//sprite_.SetIsVisible(false);

	if (!targets.empty()) {
		targets.sort([](const auto& a, const auto& b) {return a.first < b.first;});

		target_ = targets.front().second;

		transform_.translate = target_.lock().get()->GetWorldPosition();

		transform_.translate = Camera::GetInstance()->GetCameraVector3(transform_.translate, Matrix4x4::Identity());

		bool targetPush = true;

		for (TargetLockOn& target : targets_) {
			if (target.target.lock() == target_.lock()) {
				targetPush = false;
				break;
			}
		}

		if (targetPush) {
			TargetLockOn newTarget;
			newTarget.target = target_;
			newTarget.transform = transform_;
			newTarget.sprite.Initialize(TextureManager::GetInstance()->GetTextureInfo("reticle"));
			
			targets_.push_back(newTarget);
		}

		isLockOn_ = true;
	} else {
		transform_.translate.x = player.lock().get()->GetPositionReticle2D().x;
		transform_.translate.y = player.lock().get()->GetPositionReticle2D().y;
	}

	for (TargetLockOn& target : targets_) {
		if (target.target.expired()) {
			continue;
		}
		target.transform.translate = target.target.lock().get()->GetWorldPosition();
		target.transform.translate = Camera::GetInstance()->GetCameraVector3(target.transform.translate, Matrix4x4::Identity());
	}
}

void LockOn::Draw() {
	for (TargetLockOn& target : targets_) {
		if (target.target.expired()) {
			continue;
		}
		target.sprite.Draw(target.transform);
	}

	sprite_.Draw(transform_);
}

std::list<std::weak_ptr<Collider>> LockOn::GetTargets(){
	std::list<std::weak_ptr<Collider>> newTargets;

	for (TargetLockOn& target : targets_) {
		if (target.target.expired()) {
			continue;
		}
		newTargets.push_back(target.target);
	}

	return newTargets;
}
