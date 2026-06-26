#include "LockOn.h"
#include "Player.h"

void LockOn::Initialize() {
	sprite_.Initialize(TextureManager::GetInstance()->GetTextureInfo("reticle"));
	sprite_.SetColor({ 1.0f,0.0f,0.0f,1.0f });
	transform_.Initialize();

	isLockOn_ = false;
}

void LockOn::Update(std::list<std::unique_ptr<Enemy>>& enemies) {


	if (InputManager::GetInstance()->TriggerPadButton(PadButtons::INPUT_Y) || InputManager::GetInstance()->TriggerKey(DIK_LSHIFT)) {
		if (isLockOn_) {
			target_ = nullptr;
			isLockOn_ = false;
		} else {
			TargetLockOn(enemies);
		}
	}

	if (isLockOn_) {
		if (OutRange()) {
			target_ = nullptr;
			isLockOn_ = false;
		}

	}

	if (target_) {
		Vector3 positionWorld = target_->GetWorldPosition();

		Vector3 positionScreen = Camera::GetInstance()->GetCameraVector3(positionWorld, Matrix4x4::Identity());

		transform_.translate = positionScreen;
	}

	//TargetLockOn(enemies);

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
	if (!isLockOn_) {
		return;
	}

	Renderer::GetInstance()->DrawSprite(transform_, sprite_);
}

Vector3 LockOn::GetTargetPosition() const { 
	if (target_) {
		return target_->GetWorldPosition();
	} 
	
	return { 0.0f,0.0f,0.0f };
}

void LockOn::TargetLockOn(std::list<std::unique_ptr<Enemy>>& enemies) {
	std::list<std::pair<float, Enemy*>>targets;

	//Vector3 playerPos = player->GetWorldPosition();
	//playerPos = Camera::GetInstance()->GetCameraVector3(playerPos, Matrix4x4::Identity());

	for (const std::unique_ptr<Enemy>& enemy : enemies) {
		Vector3 positionWorld = enemy->GetWorldPosition();

		Vector3 positionView = Camera::GetInstance()->GetWorldViewProjectionMatrix(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, positionWorld).GetAffineMatrix()).GetMatrixToTranslate();

		if (minDistance_ <= positionView.z && positionView.z <= maxDistance_) {
			float arcTangent = std::atan2(std::sqrt(positionView.x * positionView.x + positionView.y * positionView.y), positionView.z);

			if (std::fabs(arcTangent) <= angleRange_) {
				targets.emplace_back(std::make_pair(arcTangent, enemy.get()));
			}
		}
	}

	target_ = nullptr;
	isLockOn_ = false;

	if (!targets.empty()) {
		targets.sort([](auto& pair1, auto& pair2) {return pair1.first < pair2.first; });

		target_ = targets.front().second;
		isLockOn_ = true;

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

bool LockOn::OutRange(){
	Vector3 positionWorld = target_->GetWorldPosition();

	Vector3 positionView = Camera::GetInstance()->GetWorldViewProjectionMatrix(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, positionWorld).GetAffineMatrix()).GetMatrixToTranslate();

	if (minDistance_ <= positionView.z && positionView.z <= maxDistance_) {
		float arcTangent = std::atan2(std::sqrt(positionView.x * positionView.x + positionView.y * positionView.y), positionView.z);

		if (std::fabs(arcTangent) <= angleRange_) {
			return false;
		}
	}


	return true;
}
