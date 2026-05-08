#include "Player.h"
#include "MapChipField.h"
#include <array>

void Player::Initialize(const Vector3& position) {
	transform_.Initialize();
	transform_.translate = position;
	transform_.rotate.y = Radian(90.0f);
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));

	isKirDeathAnimation_ = false;
	kirAnimationTimer_ = 0.0f;
	kirAnimationPhase_ = KirAnimationPhase::kStop;

	isDead_ = false;
}

void Player::Update() {
	if (behaviorRequest_ != Behavior::kUnknown) {
		behavior_ = behaviorRequest_;

		switch (behavior_) {
		case Player::Behavior::kRoot:
			BehaviorRootInitialize();
			break;
		case Player::Behavior::kAttack:
			BehaviorAttackInitialize();
			break;
		}

		behaviorRequest_ = Behavior::kUnknown;
	}

	switch (behavior_) {
	case Player::Behavior::kRoot:
		BehaviorRootUpdate();
		break;
	case Player::Behavior::kAttack:
		BehaviorAttackUpdate();
		break;
	}
}

void Player::BehaviorRootInitialize() {
}

void Player::BehaviorRootUpdate() {
	if (isDead_) {
		return;
	}

	if (isKirDeathAnimation_) {
		KirDeathAnimationUpdate();

		return;
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_SPACE)) {
		behaviorRequest_ = Behavior::kAttack;
	}

	MovingUpdate();

	CollisionMapInfo collisionMapInfo;

	collisionMapInfo.movementAmount = velocity_;

	MapCollision(collisionMapInfo);

	CollisionMoveUpdate(collisionMapInfo);

	CellingCollisionUpdate(collisionMapInfo);

	IsHitWallUpdate(collisionMapInfo);

	IsGroundUpdate(collisionMapInfo);

	TurningControl();

	CheckFallVoid();
}

void Player::BehaviorAttackInitialize() {
	attackParameter_ = 0.0f;
	attackPhase_ = AttackPhase::kCharge;
	velocity_ = { 0.0f ,0.0f,0.0f };

	for (uint32_t i = 0; i < 2; i++) {
		attackEffectModel_[i].Initialize(ModelManager::GetInstance()->GetModelInfo("plane"));
		attackEffectModel_[i].ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("player_attack_effect"));
		attackEffectModel_[i].SetLightingType(Renderer::LightingType::kNone);


		attackEffectTransform_[i].Initialize();
		attackEffectTransform_[i] = transform_;
	}
}

void Player::BehaviorAttackUpdate() {
	attackParameter_ += 1.0f / 60.0f;

	switch (attackPhase_) {
	case Player::AttackPhase::kCharge:
		transform_.scale.z = Easing(1.0f, 0.3f, attackParameter_, kAttackParameterCharge, EaseType::kEaseOut);
		transform_.scale.y = Easing(1.0f, 1.6f, attackParameter_, kAttackParameterCharge, EaseType::kEaseOut);

		if (attackParameter_ >= kAttackParameterCharge) {
			attackPhase_ = AttackPhase::kDash;
			attackParameter_ = 0.0f;
		}
		break;
	case Player::AttackPhase::kDash:
		transform_.scale.z = Easing(0.3f, 1.3f, attackParameter_, kAttackParameterDash, EaseType::kEaseOut);
		transform_.scale.y = Easing(1.6f, 0.7f, attackParameter_, kAttackParameterDash, EaseType::kEaseOut);

		if (attackParameter_ >= kAttackParameterDash) {
			attackPhase_ = AttackPhase::kLingeringSound;
			attackParameter_ = 0.0f;
		}

		switch (lrDirection_) {
		case Player::LRDirection::kRight:
			velocity_.x = kAttackDashSpeed;
			break;
		case Player::LRDirection::kLeft:
			velocity_.x = -kAttackDashSpeed;
			break;
		}
		break;
	case Player::AttackPhase::kLingeringSound:
		transform_.scale.z = Easing(1.3f, 1.0f, attackParameter_, kAttackParameterLingeringSound, EaseType::kEaseOut);
		transform_.scale.y = Easing(0.7f, 1.0f, attackParameter_, kAttackParameterLingeringSound, EaseType::kEaseOut);

		if (attackParameter_ >= kAttackParameterLingeringSound) {
			behaviorRequest_ = Behavior::kRoot;
			attackParameter_ = 0.0f;
		}
		break;
	}

	CollisionMapInfo collisionMapInfo;

	collisionMapInfo.movementAmount = velocity_;

	MapCollision(collisionMapInfo);

	CollisionMoveUpdate(collisionMapInfo);

	CellingCollisionUpdate(collisionMapInfo);

	IsHitWallUpdate(collisionMapInfo);

	IsGroundUpdate(collisionMapInfo);

	TurningControl();

	CheckFallVoid();

	for (uint32_t i = 0; i < 2; i++) {
		attackEffectTransform_[i].scale = { 1.2f,1.2f,1.2f };
		attackEffectTransform_[i].scale = { 1.2f,1.2f,1.2f };
		attackEffectTransform_[i].translate = transform_.translate;
		attackEffectTransform_[i].translate = transform_.translate;
		attackEffectTransform_[i].rotate = transform_.rotate;
		attackEffectTransform_[i].rotate = transform_.rotate;
	}

	attackEffectTransform_[0].rotate.z += Radian(0.0f);
	attackEffectTransform_[0].rotate.y += Radian(90.0f);
	attackEffectTransform_[1].rotate.z += Radian(180.0f);
	attackEffectTransform_[1].rotate.y += Radian(270.0f);
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
			if (velocity_.x <= 0.01f && velocity_.x >= -0.01f) {
				velocity_.x = 0.0f;
			} else {
				velocity_.x *= (1.0f - kAttenuation);
			}
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

	// 多分いらない.
	//bool landing = false;
	//if (velocity_.y < 0.0f) {
	//	if (transform_.translate.y <= 1.0f) {
	//		landing = true;
	//	}
	//}
	//
	//if (onGround_) {
	//	if (velocity_.y > 0.0f) {
	//		onGround_ = false;
	//	}
	//} else {
	//	if (landing) {
	//		transform_.translate.y = 1.0f;
	//		velocity_.x *= (1.0f - kAttenuation);
	//		velocity_.y = 0.0f;
	//		onGround_ = true;
	//	}
	//}

	//transform_.translate += velocity_;
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
	MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(transform_.translate, kLeftTop));

		if (indexSetNow.yIndex != indexSet.yIndex) {

			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.y = std::max(0.0f, (rect.bottom - transform_.translate.y) - ((kHeight / 2.0f) + kBlank));
			info.isCellingCollision = true;
		}
	}
}

void Player::MapCollisionDown(CollisionMapInfo& info) {
	if (info.movementAmount.y >= 0.0f) {
		return;
	}

	std::array<Vector3, 4> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); i++) {
		positionNew[i] = CornerPosition(transform_.translate + info.movementAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 2 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 2 - indexSet.yIndex);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(transform_.translate, kLeftBottom));

		if (indexSetNow.yIndex != indexSet.yIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.y = std::min(0.0f, (rect.top - transform_.translate.y) + ((kHeight / 2.0f) + kBlank));
			info.isLanding = true;
		}
	}
}

void Player::MapCollisionRight(CollisionMapInfo& info) {
	if (info.movementAmount.x <= 0.0f) {
		return;
	}

	std::array<Vector3, 4> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); i++) {
		positionNew[i] = CornerPosition(transform_.translate + info.movementAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	//MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(transform_.translate, kRightTop));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.x = std::max(0.0f, (rect.left - transform_.translate.x) - ((kWidth / 2.0f) + kBlank));
			info.isWallCollision = true;
		}
	}
}

void Player::MapCollisionLeft(CollisionMapInfo& info) {
	if (info.movementAmount.x >= 0.0f) {
		return;
	}

	std::array<Vector3, 4> positionNew;

	for (uint32_t i = 0; i < positionNew.size(); i++) {
		positionNew[i] = CornerPosition(transform_.translate + info.movementAmount, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	//MapChipType mapChipTypeNext;

	bool hit = false;

	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock	

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);
	// && mapChipTypeNext != MapChipType::kBlock

	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(transform_.translate, kLeftTop));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.movementAmount.x = std::min(0.0f, (rect.right - transform_.translate.x) + ((kWidth / 2.0f) + kBlank));
			info.isWallCollision = true;
		}
	}
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

void Player::IsHitWallUpdate(const CollisionMapInfo& info) {
	if (info.isWallCollision) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::IsGroundUpdate(const CollisionMapInfo& info) {
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			std::array<Vector3, 4> positionNew;

			for (uint32_t i = 0; i < positionNew.size(); i++) {
				positionNew[i] = CornerPosition(transform_.translate + info.movementAmount, static_cast<Corner>(i));
			}

			MapChipType mapChipType;

			bool hit = false;

			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kLeftBottom] + Vector3(0.0f, -kBlank, 0.0f));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);

			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionNew[kRightBottom] + Vector3(0.0f, -kBlank, 0.0f));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, mapChipField_->GetNumBlockVirtical() - 1 - indexSet.yIndex);

			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			if (!hit) {
				onGround_ = false;
			}
		}
	} else {
		if (info.isLanding) {
			onGround_ = true;
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;
		}
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

void Player::CheckFallVoid(){
	if (transform_.translate.y <= -2.0f) {
		isDead_ = true;
	}
}

void Player::KirDeathAnimationUpdate() {
	switch (kirAnimationPhase_) {
	case Player::KirAnimationPhase::kStop:
		kirAnimationTimer_ += 1.0f / 60.0f;

		if (kirAnimationTimer_ >= 0.7f) {
			kirAnimationTimer_ = 0.0f;
			velocity_ = Vector3(0.0f, kJumpAcceleration, 0.0f);
			kirAnimationPhase_ = KirAnimationPhase::kAnimation;
			SoundManager::GetInstance()->SoundPlayWave(SoundManager::GetInstance()->GetSoundData("free_k"));
		}
		break;
	case Player::KirAnimationPhase::kAnimation:

		// 落下速度.
		velocity_ += Vector3(0.0f, -kGravityAcceleration, 0.0f);
		// 速度制限.
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);

		if (kirAnimationTimer_ >= 3.0f) {
			kirAnimationTimer_ = 0.0f;
			kirAnimationPhase_ = KirAnimationPhase::kFinish;
		}

		transform_.translate += velocity_;
		break;
	case Player::KirAnimationPhase::kFinish:
		break;
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCornter] = {
		{+kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
		{-kWidth / 2.0f, -kHeight / 2.0f, 0.0f},
		{+kWidth / 2.0f, +kHeight / 2.0f, 0.0f},
		{-kWidth / 2.0f, +kHeight / 2.0f, 0.0f}
	};

	return static_cast<Vector3>(center) + offsetTable[static_cast<uint32_t>(corner)];
}

bool Player::IsAttack() const{
	if (behavior_ == Behavior::kAttack) {
		return true;
	}

	return false;
}

void Player::Draw() {
	if (isDead_) {
		return;
	}

	model_.Draw(transform_);

	if (behavior_ == Behavior::kAttack) {
		if (attackPhase_ == AttackPhase::kDash || attackPhase_ == AttackPhase::kLingeringSound) {
			for (uint32_t i = 0; i < 2; i++) {
				attackEffectModel_[i].Draw(attackEffectTransform_[i]);
			}
		}
	}
}

void Player::ScrollCollision(CollisionMapInfo& info) {
	MapCollision(info);

	CollisionMoveUpdate(info);
}

Vector3 Player::GetWorldPosition() const{
	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform_);

	Vector3 worldPos;

	worldPos.x = worldMatrix.matrix[3][0];
	worldPos.y = worldMatrix.matrix[3][1];
	worldPos.z = worldMatrix.matrix[3][2];


	return worldPos;
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f,worldPos.y - kHeight / 2.0f,worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f,worldPos.y + kHeight / 2.0f,worldPos.z + kWidth / 2.0f };

	return aabb;
}

void Player::OnCollision(const BaseEnemy* enemy) {
	(void)enemy;

	if (IsAttack()) {
		return;
	}

	isDead_ = true;
}
