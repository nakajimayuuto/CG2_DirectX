#include "Boss.h"
#include "ProjectileManager.h"

void (Boss::* Boss::pInitializeFunc[])() = {
		&Boss::WarpInitialize,
		&Boss::BulletInitialize,
		&Boss::BounsInitialize,
		&Boss::DiffusionBulletInitialize,
		&Boss::MovingBulletInitialize,
};

void (Boss::* Boss::pUpdateFunc[])() = {
		&Boss::WarpUpdate,
		&Boss::BulletUpdate,
		&Boss::BounsUpdate,
		&Boss::DiffusionBulletUpdate,
		&Boss::MovingBulletUpdate,
};

Boss::~Boss() {
	delete targetTransform_;
}

void Boss::Initialize() {
	model_.Initialize("creeking");
	halberdModel_.Initialize("halberd");
	anchorPointCenter_ = Vector3(0.0f, kBasicPositionY, 0.0f);
	anchorPoints_.push_back(anchorPointCenter_);

	for (uint32_t i = 0; i < 8; i++) {
		Vector3 pos = { 0.0f,0.5f,0.0f };
		pos.x = Rotate({ 30.0f,0.0f }, { anchorPointCenter_.x,anchorPointCenter_.z }, (i * 45.0f)).x;
		pos.z = Rotate({ 30.0f,0.0f }, { anchorPointCenter_.x,anchorPointCenter_.z }, (i * 45.0f)).y;
		anchorPoints_.push_back(anchorPointCenter_ + pos);
	}

	for (uint32_t i = 0; i < 16; i++) {
		Vector3 pos = { 0.0f,0.5f,0.0f };
		pos.x = Rotate({ 60.0f,0.0f }, { anchorPointCenter_.x,anchorPointCenter_.z }, (i * 22.5f)).x;
		pos.z = Rotate({ 60.0f,0.0f }, { anchorPointCenter_.x,anchorPointCenter_.z }, (i * 22.5f)).y;
		anchorPoints_.push_back(anchorPointCenter_ + pos);
	}

	currentAttack_ = Attacks::kWarp;

	transform_.translate.y = kBasicPositionY;
	modelTransform_.Initialize();
	modelTransform_.SetParent(&transform_);
	halberdTransform_.Initialize();
	halberdTransform_.SetParent(&transform_);
	destinationHalberdTransform_.Initialize();
	destinationHalberdTransform_.SetParent(&transform_);
	halberdTransform_.translate = kBasicHalberdFarPos;
	halberdTransform_.rotate = kBasicHalberdFarRotate;
	destinationHalberdTransform_ = halberdTransform_;
	attackRequest_ = std::nullopt;
}

void Boss::Update() {
	deltaTime_ = DeltaTime::GetInstance()->GetDeltaTime();

	ImGui::Begin("BossDebug");
	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("scale", reinterpret_cast<float*>(&transform_.scale), 0.05f, 0.0f, 5.0f);
	ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&imRotate), 1.0f, -360.0f, 360.0f);
	ImGui::DragFloat3("translate", reinterpret_cast<float*>(&transform_.translate), 0.25f, -100.0f, 100.0f);
	for (Attacks attack : magic_enum::enum_values<Attacks>()) {
		if (static_cast<size_t>(attack) == std::size(pUpdateFunc)) {
			break;
		}

		if (ImGui::Button(magic_enum::enum_name(attack).data())) {
			attackRequest_ = attack;
		}
	}


	transform_.rotate = Radian(imRotate);
	ImGui::End();

	AttackInitialize();

	AttackUpdate();

	if (!isPlayAttack_) {
		destinationAngleY_ = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		transform_.rotate.y = LerpShortAngle(transform_.rotate.y, destinationAngleY_, 0.25f);
	}

	halberdTransform_.scale = Lerp(halberdTransform_.scale, destinationHalberdTransform_.scale, kDestinationCompletionRate);
	halberdTransform_.rotate = LerpShortAngle(halberdTransform_.rotate, destinationHalberdTransform_.rotate, kDestinationCompletionRate);
	halberdTransform_.translate = Lerp(halberdTransform_.translate, destinationHalberdTransform_.translate, kDestinationCompletionRate);

}

void Boss::Draw() {
	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawModel(modelTransform_, &model_, true);
	renderer->DrawShadow(modelTransform_, &model_, { 0.0f,0.0f,0.0f,1.0f });

	renderer->DrawModel(halberdTransform_, &halberdModel_, true);
	renderer->DrawShadow(halberdTransform_, &halberdModel_, { 0.0f,0.0f,0.0f,1.0f });

	for (Vector3& pos : anchorPoints_) {
		renderer->DrawSphereWireFrame(Transform::GetInitialValue({ 0.1f,0.1f,0.1f }, { 0.0f,0.0f,0.0f }, pos), { 0.5f,0.5f,1.0f,1.0f });
	}
}

void Boss::OnCollision(Collider* other) {
}

Vector3 Boss::GetMoveAnchorPointFindAll(){
	float maxLength = 0.0f;
	Vector3 newPos = { 0.0f,0.0f,0.0f };
	for (Vector3& pos : anchorPoints_) {
		float newLength = (pos - targetTransform_->translate).Length();
		if (newLength >= maxLength) {
			maxLength = newLength;
			newPos = pos;
		}
	}
	return newPos;
}

Vector3 Boss::GetMoveAnchorPointFind(float radius){
	float maxLength = 0.0f;
	Vector3 newPos = transform_.translate;
	for (Vector3& pos : anchorPoints_) {
		if ((pos - transform_.translate).Length() >= radius) {
			continue;
		}

		float newLength = (pos - targetTransform_->translate).Length();
		if (newLength >= maxLength) {
			maxLength = newLength;
			newPos = pos;
		}
	}
	return newPos;
}

void Boss::AttackInitialize() {
	if (isPlayAttack_ || !attackRequest_) {
		return;
	}

	transform_.rotate.y = destinationAngleY_;
	currentAttack_ = attackRequest_.value();
	//attackRequest_ = std::nullopt;
	isPlayAttack_ = true;
	currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	kMaxAttackTimer = 0.0f; // 攻撃のタイマー最大値.
	currentAttackPhase = 0; // 攻撃のフェーズ.
	preTransform_ = transform_;

	(this->*pInitializeFunc[static_cast<size_t>(currentAttack_)])();
}

void Boss::AttackUpdate() {
	if (!isPlayAttack_) {
		return;
	}

	(this->*pUpdateFunc[static_cast<size_t>(currentAttack_)])();

	currentAttackTimer_ += deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
}

void Boss::AttackFinished() {
	isPlayAttack_ = false;
	attackRequest_ = std::nullopt;
	currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	kMaxAttackTimer = 0.0f; // 攻撃のタイマー最大値.
	currentAttackPhase = 0; // 攻撃のフェーズ.
	SetCurrentDistanceHalberdTransform();
}

void Boss::NextAttackPhase(float timerMax) {
	currentAttackTimer_ = 0;
	kMaxAttackTimer = timerMax;
	currentAttackPhase++;
}

void Boss::SetCurrentDistanceHalberdTransform() {
	destinationHalberdTransform_.translate = kBasicHalberdFarPos;
	destinationHalberdTransform_.rotate = kBasicHalberdFarRotate;
}

void Boss::WarpInitialize() {
	kMaxAttackTimer = kWarpEnterTimerMax;
}

void Boss::WarpUpdate() {
	switch (currentAttackPhase) {
	case 0:
		transform_.scale = Easing({ 1.0f,1.0f,1.0f }, { 2.0f,0.0f,2.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			currentAttackTimer_ = 0;
			kMaxAttackTimer = kWarpFinishedTimerMax;
			currentAttackPhase = 1;
			transform_.translate = GetMoveAnchorPointFindAll();
			//transform_.translate = GetMoveAnchorPointFind(60.0f);
		}

		break;
	case 1:
		transform_.scale = Easing({ 0.0f,2.0f,0.0f }, { 1.0f,1.0f,1.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::BulletInitialize() {
	kMaxAttackTimer = kBulletStartGapTimerMax;
	bulletShotDirectionTemp_ = { 0.0f,0.0f,1.0f };
}

void Boss::BulletUpdate() {
	switch (currentAttackPhase) {
	case 0:
		modelTransform_.rotate.y = Easing(0.0f, Radian(kBulletAnimRotateY), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBasicHalberdFarPos, kBulletHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBulletStayTimerMax);
			if ((targetTransform_->translate - transform_.translate).Length() != 0.0f) {
				bulletShotDirectionTemp_ = (targetTransform_->translate - transform_.translate).Normalize();
			}
		}
		break;
	case 1:
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBulletStayTimerMax);
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, bulletShotDirectionTemp_ * 30.0f, BulletType::kNormal);
		}
		break;
	case 2:
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBulletFinishedGapTimerMax);
		}
		break;
	case 3:
		modelTransform_.rotate.y = Easing(Radian(kBulletAnimRotateY), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBulletHalberdPos, kBasicHalberdFarPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::BounsInitialize() {
	kMaxAttackTimer = kBounsStartGapTimerMax;
}

void Boss::BounsUpdate() {
	float randomRadian;
	switch (currentAttackPhase) {
	case 0:
		transform_.translate.y = Easing(kBasicPositionY, kBounsAnimPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.translate = Easing(kBasicHalberdFarPos, kBounsHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kBasicHalberdFarRotate, kBounsHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBounsStayTimerMax);
		}
		break;
	case 1:
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBounsSpinTimerMax);
		}
		break;
	case 2:
		transform_.rotate.y = Easing(preTransform_.rotate.y, preTransform_.rotate.y +  Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBounsHalberdStartPos, kBounsHalberdSpinPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kBounsHalberdStartRotate, kBounsHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBounsSpinTimerMax);
			randomRadian = Radian(Random::GetInstance()->RandomFloat(0.0f, 359.0f));
			ProjectileManager::GetInstance()->CreateDiffusionBullet(transform_, Vector3(RadianToVector(randomRadian).x, 0.0f, RadianToVector(randomRadian).y) * 10.0f, BulletType::kBounce, Radian(45.0f), 8);
		}
		break;
	case 3:
		transform_.rotate.y = Easing(preTransform_.rotate.y, preTransform_.rotate.y + Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBounsFinishedGapTimerMax);
			transform_.rotate.y = preTransform_.rotate.y;
		}
		break;
	case 4:
		transform_.translate.y = Easing(kBounsAnimPositionY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBounsHalberdSpinPos, kBasicHalberdFarPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(kBounsHalberdSpinRotate, kBasicHalberdFarRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::DiffusionBulletInitialize() {
	kMaxAttackTimer = kDiffusionBulletStartGapTimerMax;
}

void Boss::DiffusionBulletUpdate() {
	switch (currentAttackPhase) {
	case 0:
		destinationHalberdTransform_.translate = Easing(kBasicHalberdFarPos, kDiffusionBulletHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kBasicHalberdFarRotate, kDiffusionBulletHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletSpinTimerMax);
		}
		break;
	case 1:
		destinationHalberdTransform_.rotate = Easing(kDiffusionBulletHalberdStartRotate, kDiffusionBulletHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletBackTimerMax);
			Vector3 direction = { 0.0f,0.0f,-1.0f };
			if ((targetTransform_->translate - transform_.translate).Length() != 0.0f) {
				direction = (targetTransform_->translate - transform_.translate).Normalize();
			}
			ProjectileManager::GetInstance()->CreateDiffusionBullet(halberdTransform_, direction * 30.0f, BulletType::kNormal, Radian(30.0f), 3);

		}
		break;
	case 2:
		modelTransform_.translate.z = Easing(0.0f,kDiffusionBulletAnimPositionZ, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletBackTimerMax);
		}
		break;
	case 3:
		modelTransform_.translate.z = Easing(kDiffusionBulletAnimPositionZ, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletFinishedGapTimerMax);
		}
		break;
	case 4:
		destinationHalberdTransform_.translate = Easing(kDiffusionBulletHalberdStartPos, kBasicHalberdFarPos,  currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kDiffusionBulletHalberdStartRotate, kBasicHalberdFarRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::MovingBulletInitialize(){
	kMaxAttackTimer = kMovingBulletStartGapTimerMax;
	movingBulletTimer_ = 0.0f;
	movingBulletTargetPos = GetMoveAnchorPointFind(kMovingBulletAnchorRadius);
}

void Boss::MovingBulletUpdate(){
	movingBulletTimer_ += deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
	transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
	Vector3 direction = { 0.0f,0.0f,-1.0f };
	transform_.translate = Easing(preTransform_.translate, movingBulletTargetPos, movingBulletTimer_, kMovingBulletFinishedTimerMax, EaseType::kEaseOut);

	switch (currentAttackPhase) {
	case 0:
		modelTransform_.rotate.y = Easing(0.0f, Radian(kBulletAnimRotateY), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBasicHalberdFarPos, kBulletHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMovingBulletStayTimerMax);
		}
		break;
	case 1:
	case 2:
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMovingBulletShotGapTimerMax);
			if ((targetTransform_->translate - transform_.translate).Length() != 0.0f) {
				direction = (targetTransform_->translate - transform_.translate).Normalize();
			}
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, direction * 30.0f, BulletType::kNormal);
		}
		break;
	case 3:
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMovingBulletFinishedGapTimerMax);
			if ((targetTransform_->translate - transform_.translate).Length() != 0.0f) {
				direction = (targetTransform_->translate - transform_.translate).Normalize();
			}
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, direction * 30.0f, BulletType::kNormal);
		}
		break;
	case 4:
		modelTransform_.rotate.y = Easing(Radian(kBulletAnimRotateY), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBulletHalberdPos, kBasicHalberdFarPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMovingBulletFinishedGapTimerMax);
		}
		break;
	case 5:
		break;
	}

	if (movingBulletTimer_ >= kMovingBulletFinishedTimerMax) {
		AttackFinished();
	}
}
