#include "Player.h"
#include "MapChipField.h"
#include <array>

void Player::Initialize(const Vector3& position) {
	transform_.Initialize();
	transform_.translate = position;
	transform_.rotate.y = Radian(90.0f);
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
}

void Player::Update() {
	MovingUpdate();

	CollisionMapInfo collisionMapInfo;

	collisionMapInfo.movementAmount = velocity_;

	MapCollision(collisionMapInfo);

	CollisionMoveUpdate(collisionMapInfo);

	CellingCollisionUpdate(collisionMapInfo);

	TurningControl();
}

void Player::MovingUpdate() {
	if (onGround_) {
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
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
		} else {
			// 非入力時は移動減衰をかける.
			velocity_.x *= (1.0f - kAttenuation);
		}

		if (InputManager::GetInstance()->PressKey(DIK_UP)) {
			velocity_ += Vector3(0.0f, kJumpAcceleration, 0.0f);
		}
	} else {
		// 落下速度.
		velocity_ += Vector3(0.0f, -kGravityAcceleration, 0.0f);
		// 速度制限.
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}

	bool landing = false;
	if (velocity_.y < 0.0f) {
		if (transform_.translate.y <= 1.0f) {
			landing = true;
		}
	}

	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		}
	} else {
		if (landing) {
			transform_.translate.y = 1.0f;
			velocity_.x *= (1.0f - kAttenuation);
			velocity_.y = 0.0f;
			onGround_ = true;
		}
	}

	transform_.translate += velocity_;
}

void Player::MapCollision(CollisionMapInfo& info) {
	MapCollisionUp(info);
	MapCollisionDown(info);
	MapCollisionRight(info);
	MapCollisionLeft(info);
}

void Player::MapCollisionUp(CollisionMapInfo& info) {
	if (info.movementAmount.y <= 0.0f) {
		return;
	}

	std::array<Vector3, 4> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); i++) {
		positionNew[i] = CornerPosition(transform_.translate + info.movementAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);

		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.movementAmount.y = std::max(0.0f, (rect.bottom - transform_.translate.y) - ((kHeight / 2.0f) + kBlank));
		info.isCellingCollision = true;
	}
}

void Player::MapCollisionDown(CollisionMapInfo& info) {
}

void Player::MapCollisionRight(CollisionMapInfo& info) {
}

void Player::MapCollisionLeft(CollisionMapInfo& info) {
}

void Player::CollisionMoveUpdate(const CollisionMapInfo& info) {
	transform_.translate += info.movementAmount;
}

void Player::CellingCollisionUpdate(const CollisionMapInfo& info) {
	if (info.isCellingCollision) {
		GameSystem::GetInstance()->Log("hit ceiling\n");
		velocity_.y = 0.0f;
	}
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

		transform_.rotate.y = Easing(turnFirstRotationY_, destinationRotationY, kTimeTurn - turnTimer_, kTimeTurn, EaseType::kConstant);
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCornter] = {
		{-kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
		{+kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
		{-kWidth / 2.0f, +kHeight / 2.0f, 0.0f},
		{+kWidth / 2.0f, +kHeight / 2.0f, 0.0f}
	};

	return static_cast<Vector3>(center) + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::Draw() {
	model_.Draw(transform_);
}
