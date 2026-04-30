#include "Player.h"

void Player::Initialize(const Vector3& position) {
	transform_.Initialize();
	transform_.translate = position;
	transform_.rotate.y = Radian(90.0f);
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
}

void Player::Update() {
	if (InputManager::GetInstance()->PressKey(DIK_RIGHT) || InputManager::GetInstance()->PressKey(DIK_LEFT)) {
		// 左右加速.
		Vector3 acceleration = {};

		if (InputManager::GetInstance()->PressKey(DIK_RIGHT)) {
			// 左移動中の右入力.
			if (velocity_.x < 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}

			acceleration.x += kAcceletation;

			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;

				turnFirstRotationY_ = transform_.rotate.y;
				turnTimer_ = kTimeTurn;
			}
		} else if (InputManager::GetInstance()->PressKey(DIK_LEFT)) {
			// 右移動中の左入力.
			if (velocity_.x > 0.0f) {
				velocity_.x *= (1.0f - kAttenuation);
			}

			acceleration.x -= kAcceletation;

			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;

				turnFirstRotationY_ = transform_.rotate.y;
				turnTimer_ = kTimeTurn;
			}
		}

		// 加速減速.
		velocity_ += acceleration;

		// 最大速度制限.
		velocity_.x = std::clamp(velocity_.x,-kLimitRunSpeed,kLimitRunSpeed);
	} else {
		// 非入力時は移動減衰をかける.
		velocity_.x *= (1.0f - kAttenuation);
	}

	transform_.translate += velocity_;

	TurningControl();
}

void Player::TurningControl() {
	// 旋回制御.
	
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル.
		float destinationRotationYTable[] = {
			Radian(90.0f),
			Radian(270.0f),
		};

		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		transform_.rotate.y = Easing(turnFirstRotationY_,destinationRotationY,kTimeTurn - turnTimer_,kTimeTurn,EaseType::kConstant);
	}
}

void Player::Draw() {
	model_.Draw(transform_);
}
