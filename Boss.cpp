#include "Boss.h"
#include "ProjectileManager.h"
#include "GameCamera.h"

void (Boss::* Boss::pInitializeFunc[])() = {
		&Boss::WarpInitialize,
		&Boss::BulletInitialize,
		&Boss::BounsInitialize,
		&Boss::DiffusionBulletInitialize,
		&Boss::MovingBulletInitialize,
		&Boss::WaveInitialize,
		&Boss::SpinningInitialize,
		&Boss::PowerSlasherInitialize,
		&Boss::FangAttackInitialize,
		&Boss::NearAttackInitialize,
};

void (Boss::* Boss::pUpdateFunc[])() = {
		&Boss::WarpUpdate,
		&Boss::BulletUpdate,
		&Boss::BounsUpdate,
		&Boss::DiffusionBulletUpdate,
		&Boss::MovingBulletUpdate,
		&Boss::WaveUpdate,
		&Boss::SpinningUpdate,
		&Boss::PowerSlasherUpdate,
		&Boss::FangAttackUpdate,
		&Boss::NearAttackUpdate,
};

Boss::~Boss() {
	delete targetTransform_;
}

void Boss::Initialize() {
	model_.Initialize("boss");
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
	halberdTransform_.translate = basicHalberdPos;
	halberdTransform_.rotate = basicHalberdRotate;
	destinationHalberdTransform_ = halberdTransform_;
	attackRequest_ = std::nullopt;

	colliderType_ = ColliderType::kBox;
	colliderSize_ = kBasicColliderSize;
	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy);
	collisionMask_ = CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack);

	colliderColor_ = { 0.6f,0.3f,1.0f,1.0f };

	attackTempTransform_.Initialize();
	attackTempCollider_ = std::make_unique<Collider>();
	attackTempTransform_.SetParent(&transform_);
	attackTempCollider_->SetRadius(2.0f);
	attackTempCollider_->SetColliderType(ColliderType::kSphere);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy));
	attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });

	maxHP_ = 1000.0f;
	currentHP_ = maxHP_;

	hpGauge = std::make_unique<HPGauge>();
	hpGauge->Initialize(&currentHP_, maxHP_, { 800.0f,60.0f });
	hpGauge->SetPosition({ 0.0f,-300.0f });

	DifficultyManager::GetInstance()->SetBossHPData(&currentHP_, maxHP_);

	GameCamera::GetInstance()->SetEnemyTransform(&transform_);

	Phase1Initialize();

#ifdef _DEBUG
	useDebugUpdateStop = true;
#endif // _DEBUG

}

void Boss::SetAttackData(Attacks attackName, float weight, DistanceName name) {
	AttackData newData;
	newData.attackName = attackName;
	newData.continuousCount = 0;
	newData.weight = weight;
	newData.magnification = 1.0f;
	switch (name) {
	case Boss::DistanceName::kNear:
		nearAttackDatas_.push_back(newData);
		break;
	case Boss::DistanceName::kMiddle:
		middleAttackDatas_.push_back(newData);
		break;
	case Boss::DistanceName::kFar:
		farAttackDatas_.push_back(newData);
		break;
	}
}

void Boss::Phase1Initialize() {
	nearAttackDatas_.clear();
	middleAttackDatas_.clear();
	farAttackDatas_.clear();

	SetAttackData(Attacks::kBulletShot, 1.0f, DistanceName::kFar);
	SetAttackData(Attacks::kDiffusionShot, 0.5f, DistanceName::kFar);
	SetAttackData(Attacks::kMovingShot, 0.5f, DistanceName::kFar);

	SetAttackData(Attacks::kBounsShot, 0.5f, DistanceName::kMiddle);
	SetAttackData(Attacks::kFangAttack, 0.5f, DistanceName::kMiddle);
	SetAttackData(Attacks::kPowerSlasher, 0.5f, DistanceName::kMiddle);
	SetAttackData(Attacks::kSpinningHalberd, 0.5f, DistanceName::kMiddle);

	SetAttackData(Attacks::kNearAttack, 0.5f, DistanceName::kNear);
	SetAttackData(Attacks::kMovingShot, 0.5f, DistanceName::kNear);
	SetAttackData(Attacks::kPowerSlasher, 0.5f, DistanceName::kNear);

}

void Boss::Update() {
	dopamineSpeed_ = DifficultyManager::GetInstance()->GetDopamineSpeed();

	difficultyMagnificationTime = DifficultyManager::GetInstance()->GetSpeedMagnification();

	deltaTime_ = DeltaTime::GetInstance()->GetGameTime();

	DistanceCheckUpdate();

	HalberdStanceUpdate();

#ifdef _DEBUG

	ImGui::Begin("BossDebug");

	if (ImGui::Button("useUpdateStop")) {
		if (useDebugUpdateStop) {
			useDebugUpdateStop = false;
		} else {
			useDebugUpdateStop = true;
			AttackFinished();
		}
	}
	ImGui::Text("&s", useDebugUpdateStop ? "true" : "false");


	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("scale", reinterpret_cast<float*>(&transform_.scale), 0.05f, 0.0f, 5.0f);
	ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&imRotate), 1.0f, -360.0f, 360.0f);
	ImGui::DragFloat3("translate", reinterpret_cast<float*>(&transform_.translate), 0.25f, -100.0f, 100.0f);
	ImGui::DragFloat("HP", &currentHP_, 1.0f, 0.0f, maxHP_);
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
#endif // _DEBUG

	if (InputManager::GetInstance()->TriggerKey(DIK_P)) {
		attackRequest_ = Attacks::kFangAttack;
	}

	RootUpdate();

	AttackInitialize();

	AttackUpdate();



	halberdTransform_.scale = Lerp(halberdTransform_.scale, destinationHalberdTransform_.scale, kDestinationCompletionRate);
	halberdTransform_.rotate = LerpShortAngle(halberdTransform_.rotate, destinationHalberdTransform_.rotate, kDestinationCompletionRate);
	halberdTransform_.translate = Lerp(halberdTransform_.translate, destinationHalberdTransform_.translate, kDestinationCompletionRate);

	CollisionManager::GetInstance()->AddColliderList(this);
}

void Boss::DistanceCheckUpdate() {
	Vector3 lenght = targetTransform_->translate - transform_.translate;
	if (lenght.Length() <= kNearRadius) {
		currentDistance_ = DistanceName::kNear;
	} else if (lenght.Length() <= kMiddleRadius) {
		currentDistance_ = DistanceName::kMiddle;
	} else {
		currentDistance_ = DistanceName::kFar;
	}
}

void Boss::RootUpdate() {
	if (isPlayAttack_) {
		return;
	}

	if (!useDebugUpdateStop) {
		attackCoolTimer_ += deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
	}

	destinationAngleY_ = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
	transform_.rotate.y = LerpShortAngle(transform_.rotate.y, destinationAngleY_, 0.25f);

	destinationHalberdTransform_.rotate = basicHalberdRotate;
	destinationHalberdTransform_.translate = basicHalberdPos;

	if (attackCoolTimer_ >= attackCoolTimeMax_) {
		attackCoolTimeMax_ = 2.0f;
		attackCoolTimer_ = 0.0f;
		switch (currentDistance_) {
		case Boss::DistanceName::kNear:
			AttackSelect(nearAttackDatas_);
			break;
		case Boss::DistanceName::kMiddle:
			AttackSelect(middleAttackDatas_);
			break;
		case Boss::DistanceName::kFar:
			AttackSelect(farAttackDatas_);
			break;
		}
	}
}

void Boss::AttackSelect(std::vector<AttackData> attackDatas) {
	std::vector<std::pair<Attacks, float>> randomData;
	float weightMax = 0.0f;
	float selectNum;

	for (AttackData& data : attackDatas) {
		float weight = data.weight * std::pow(0.5f, static_cast<float>(data.continuousCount)) * data.magnification * 10000.0f;

		weightMax = weight + weightMax;

		randomData.push_back(std::pair<Attacks, float>(data.attackName, weight));
	}

	selectNum = Random::GetInstance()->RandomFloat(1.0f, weightMax);

	weightMax = 0.0f;

	for (std::pair<Attacks, float>& data : randomData) {
		weightMax += data.second;
		if (selectNum < weightMax) {
			attackRequest_ = data.first;
			break;
		}
	}

	ClearAttackDatas();
}

void Boss::ClearAttackDatas() {
	for (AttackData& data : nearAttackDatas_) {
		data.magnification = 1.0f;
		if (data.attackName == attackRequest_) {
			if (data.continuousCount < 3) {
				data.continuousCount++;
			}
		} else {
			data.continuousCount = 0;
		}
	}

	for (AttackData& data : middleAttackDatas_) {
		data.magnification = 1.0f;
		if (data.attackName == attackRequest_) {
			if (data.continuousCount < 3) {
				data.continuousCount++;
			}
		} else {
			data.continuousCount = 0;
		}
	}

	for (AttackData& data : farAttackDatas_) {
		data.magnification = 1.0f;
		if (data.attackName == attackRequest_) {
			if (data.continuousCount < 3) {
				data.continuousCount++;
			}
		} else {
			data.continuousCount = 0;
		}
	}
}

void Boss::HalberdStanceUpdate() {
	switch (currentDistance_) {
	case Boss::DistanceName::kNear:
		basicHalberdPos = kBasicHalberdNearPos;
		basicHalberdRotate = kBasicHalberdNearRotate;
		break;
	case Boss::DistanceName::kMiddle:
		basicHalberdPos = kBasicHalberdMiddlePos;
		basicHalberdRotate = kBasicHalberdMiddleRotate;
		break;
	case Boss::DistanceName::kFar:
		basicHalberdPos = kBasicHalberdFarPos;
		basicHalberdRotate = kBasicHalberdFarRotate;
		break;
	}
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

	DrawCollider();

	hpGauge->Draw();
}

void Boss::OnCollision(Collider* other) {
	if ((other->GetCollisionAttribute() & CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack)) == 0x0) {
		currentHP_ -= other->GetDamage();

		DeltaTime::GetInstance()->SetHitStop(other->GetDamage() * 0.01f);

		if (currentHP_ < 0.0f) {
			currentHP_ = 0.0f;
		}
	}
}

Vector3 Boss::GetMoveAnchorPointFindAll() {
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

Vector3 Boss::GetMoveAnchorPointFind(float radius) {
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
	isColliderActive_ = true;
	attackRequest_ = std::nullopt;
	currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	kMaxAttackTimer = 0.0f; // 攻撃のタイマー最大値.
	currentAttackPhase = 0; // 攻撃のフェーズ.
	attackTempCollider_->SetDamageCoolTime(-1.0f);
	attackTempCollider_->SetDamage(10.0f);
	halberdTransform_.SetParent(&transform_);
	SetCurrentDistanceHalberdTransform();
}

void Boss::NextAttackPhase(float timerMax) {
	currentAttackTimer_ = 0;
	kMaxAttackTimer = timerMax;
	currentAttackPhase++;
}

void Boss::SetCurrentDistanceHalberdTransform() {
	destinationHalberdTransform_.translate = basicHalberdPos;
	destinationHalberdTransform_.rotate = basicHalberdRotate;
}

void Boss::WarpInitialize() {
	kMaxAttackTimer = kWarpEnterTimerMax;
}

void Boss::WarpUpdate() {
	switch (currentAttackPhase) {
	case 0: // ワープの始まり.
		transform_.scale = Easing({ 1.0f,1.0f,1.0f }, { 2.0f,0.0f,2.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			currentAttackTimer_ = 0;
			kMaxAttackTimer = kWarpFinishedTimerMax;
			currentAttackPhase = 1;
			transform_.translate = GetMoveAnchorPointFindAll();
			//transform_.translate = GetMoveAnchorPointFind(60.0f);
		}

		break;
	case 1: // ワープの終わり.
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
	case 0: // ハルバードを前に向ける.
		modelTransform_.rotate.y = Easing(0.0f, Radian(kBulletAnimRotateY), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kBulletHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kBasicHalberdFarRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBulletStayTimerMax);
			if ((targetTransform_->translate - halberdTransform_.GetWorldPosition()).Length() != 0.0f) {
				bulletShotDirectionTemp_ = (targetTransform_->translate - halberdTransform_.GetWorldPosition()).Normalize();
			}
		}
		break;
	case 1: // 攻撃の前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBulletStayTimerMax);
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, bulletShotDirectionTemp_ * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f);
		}
		break;
	case 2: // 攻撃の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBulletFinishedGapTimerMax);
		}
		break;
	case 3: // 見た目を戻す.
		modelTransform_.rotate.y = Easing(Radian(kBulletAnimRotateY), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBulletHalberdPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(kBasicHalberdFarRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
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
	case 0: // 上昇.
		transform_.translate.y = Easing(kBasicPositionY, kBounsAnimPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kBounsHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kBounsHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBounsStayTimerMax);
		}
		break;
	case 1: // 上空で間を開ける.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBounsSpinTimerMax);
		}
		break;
	case 2: // 回転し初め、遷移時に攻撃を発射.
		transform_.rotate.y = Easing(preTransform_.rotate.y, preTransform_.rotate.y + Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBounsHalberdStartPos, kBounsHalberdSpinPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kBounsHalberdStartRotate, kBounsHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBounsSpinTimerMax);
			randomRadian = Radian(Random::GetInstance()->RandomFloat(0.0f, 359.0f));
			ProjectileManager::GetInstance()->CreateDiffusionBullet(transform_, Vector3(RadianToVector(randomRadian).x, 0.0f, RadianToVector(randomRadian).y) * 10.0f, BulletType::kBounce, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(45.0f), 8);
		}
		break;
	case 3: // 回転を終了する.
		transform_.rotate.y = Easing(preTransform_.rotate.y, preTransform_.rotate.y + Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kBounsFinishedGapTimerMax);
			transform_.rotate.y = preTransform_.rotate.y;
		}
		break;
	case 4: // 降下.
		transform_.translate.y = Easing(kBounsAnimPositionY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBounsHalberdSpinPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(kBounsHalberdSpinRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
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
	case 0: // ハルバードを前に構える.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kDiffusionBulletHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kDiffusionBulletHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletSpinTimerMax);
		}
		break;
	case 1: // ハルバードを高速回転させる.
		destinationHalberdTransform_.rotate = Easing(kDiffusionBulletHalberdStartRotate, kDiffusionBulletHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletBackTimerMax);
			Vector3 direction = { 0.0f,0.0f,-1.0f };
			if ((targetTransform_->translate - transform_.translate).Length() != 0.0f) {
				direction = (targetTransform_->translate - transform_.translate).Normalize();
			}
			ProjectileManager::GetInstance()->CreateDiffusionBullet(halberdTransform_, direction * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(30.0f), 3);

		}
		break;
	case 2: // 少し後退
		modelTransform_.translate.z = Easing(0.0f, kDiffusionBulletAnimPositionZ, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletBackTimerMax);
		}
		break;
	case 3: // 元の位置に戻る.
		modelTransform_.translate.z = Easing(kDiffusionBulletAnimPositionZ, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletFinishedGapTimerMax);
		}
		break;
	case 4: // 見た目を戻す.
		destinationHalberdTransform_.translate = Easing(kDiffusionBulletHalberdStartPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kDiffusionBulletHalberdStartRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::MovingBulletInitialize() {
	kMaxAttackTimer = kMovingBulletStartGapTimerMax;
	movingBulletTimer_ = 0.0f;
	movingBulletTargetPos = GetMoveAnchorPointFind(kMovingBulletAnchorRadius);
}

void Boss::MovingBulletUpdate() {
	movingBulletTimer_ += deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
	transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
	Vector3 direction = { 0.0f,0.0f,-1.0f };
	transform_.translate = Easing(preTransform_.translate, movingBulletTargetPos, movingBulletTimer_, kMovingBulletFinishedTimerMax, EaseType::kEaseOut);

	switch (currentAttackPhase) {
	case 0:// ハルバードを構える.
		modelTransform_.rotate.y = Easing(0.0f, Radian(kBulletAnimRotateY), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kBulletHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kBasicHalberdFarRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMovingBulletStayTimerMax);
		}
		break;
	case 1:// 攻撃を発射させる間.
	case 2:
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMovingBulletShotGapTimerMax);
			if ((targetTransform_->translate - destinationHalberdTransform_.GetWorldPosition()).Length() != 0.0f) {
				direction = (targetTransform_->translate - destinationHalberdTransform_.GetWorldPosition()).Normalize();
			}
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, direction * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 0.5f);
		}
		break;
	case 3: // 攻撃を発射させる間(3回目).
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMovingBulletFinishedGapTimerMax);
			if ((targetTransform_->translate - destinationHalberdTransform_.GetWorldPosition()).Length() != 0.0f) {
				direction = (targetTransform_->translate - destinationHalberdTransform_.GetWorldPosition()).Normalize();
			}
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, direction * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 0.5f);
		}
		break;
	case 4: // 見た目を戻す.
		modelTransform_.rotate.y = Easing(Radian(kBulletAnimRotateY), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBulletHalberdPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(kBasicHalberdFarRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMovingBulletFinishedGapTimerMax);
		}
		break;
	case 5:
		break;
	}

	if (movingBulletTimer_ >= kMovingBulletFinishedTimerMax) {
		// 一定の時間経過後攻撃終了.
		AttackFinished();
	}
}

void Boss::WaveInitialize() {
	kMaxAttackTimer = kMovingBulletStartGapTimerMax;
	movingBulletTimer_ = 0.0f;
	movingBulletTargetPos = GetMoveAnchorPointFind(kMovingBulletAnchorRadius);
}

void Boss::WaveUpdate() {

	switch (currentAttackPhase) {
	case 0: // 上昇しながらハルバードを前に構える.
		transform_.translate.y = Easing(kBasicPositionY, kWaveAnimPositionY / 2.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kWaveHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kWaveHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveStartGapTimerMax);
			halberdTransform_.SetParent(&modelTransform_);
		}
		break;
	case 1: // 残りの上昇.
		transform_.translate.y = Easing(kWaveAnimPositionY / 2.0f, kWaveAnimPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.x = Easing(kWaveHalberdStartRotate.x, preTransform_.rotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kWaveHalberdPos, kWaveHalberdSpinPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveSpinTimerMax);
			preTransform_ = transform_;
		}
		break;
	case 2: // その場で回転.
		transform_.rotate.x = Easing(preTransform_.rotate.x, preTransform_.rotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveAttackTimerMax);
		}
		break;
	case 3: // 攻撃態勢に入りながら急降下.
		transform_.translate.y = Easing(kWaveAnimPositionY, kWaveAttackPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		transform_.rotate.x = Easing(preTransform_.rotate.x, kWaveAttackRotateX, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kWaveHalberdSpinPos, kWaveHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveAttackGapTimerMax);
			ProjectileManager::GetInstance()->CreateWave(transform_, 25.0f, 1.0f, -1.0f, kCollisionEnemyAttack, 15.0f, 3.0f);
		}
		break;
	case 4: // 攻撃後の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveFinishedGapTimerMax);
		}
		break;
	case 5: // 見た目を戻す.
		transform_.translate.y = Easing(kWaveAttackPositionY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		transform_.rotate.x = Easing(kWaveAttackRotateX, preTransform_.rotate.x, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.translate = Easing(kWaveHalberdAttackPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			halberdTransform_.SetParent(&transform_);
		}
		break;
	}
}

void Boss::SpinningInitialize() {
	kMaxAttackTimer = kSpinningStartGapTimerMax;
	attackTempTransform_.translate = { 1.0f,0.0f,0.0f };
	attackTempTransform_.SetParent(&transform_);
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);
	attackTempCollider_->SetColliderType(ColliderType::kBox);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy));
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	attackTempCollider_->SetDamage(5.0f);
	attackTempCollider_->SetDamageCoolTime(0.1f);
	spinningRotateY = 0.0f;
	isColliderActive_ = false;
}

void Boss::SpinningUpdate() {
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);

	Vector3 move = { 0.0f,0.0f,kSpinningSpeed };

	spinningRotateY = LerpShortAngle(spinningRotateY, std::atan2(targetTransform_->translate.x - transform_.translate.x, targetTransform_->translate.z - transform_.translate.z), kSpinningComplateRate);

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix({ 0.0f,spinningRotateY,0.0f });

	move = rotateMatrix.TransformNomal(move);

	switch (currentAttackPhase) {
	case 0: // ハルバードを構える.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kSpinningHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kSpinningHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = Easing(preTransform_.rotate.y, preTransform_.rotate.y + kSpinningStartRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kSpinningStayTimerMax);
		}
		break;
	case 1: // 前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kSpinningSpinStartTimerMax);
		}
		break;
	case 2: // 回転し初め.
		transform_.rotate.y = Easing(preTransform_.rotate.y + kSpinningStartRotateY, preTransform_.rotate.y + kSpinningStartRotateY - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kSpinningSpinTimerMax);
			transform_.rotate.y = preTransform_.rotate.y + kSpinningStartRotateY;
		}
		break;
	case 3: // 回転の最中.
		transform_.rotate.y = Easing(preTransform_.rotate.y + kSpinningStartRotateY, preTransform_.rotate.y + kSpinningSpinGapRotateY - (Radian(360.0f) * 10.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kSpinningSpinFinnishedTimerMax);
			transform_.rotate.y = preTransform_.rotate.y + kSpinningSpinGapRotateY;
		}
		break;
	case 4: // 回転し終わり.
		transform_.rotate.y = Easing(preTransform_.rotate.y + kSpinningSpinGapRotateY, preTransform_.rotate.y + kSpinningSpinGapRotateY - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.translate = Easing(kSpinningHalberdStartPos, kSpinningHalberdSpinGapPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kSpinningHalberdStartRotate, kSpinningHalberdSpinGapRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kSpinningStayTimerMax);
			transform_.rotate.y = preTransform_.rotate.y + kSpinningSpinGapRotateY;
		}
		break;
	case 5: // 後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveFinishedGapTimerMax);
		}
		break;
	case 6: // 見た目を戻す.
		destinationHalberdTransform_.translate = Easing(kSpinningHalberdSpinGapPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kSpinningHalberdSpinGapRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = Easing(preTransform_.rotate.y + kSpinningSpinGapRotateY, preTransform_.rotate.y, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}


	attackTempTransform_ = destinationHalberdTransform_;
	if (currentAttackPhase <= 4 && currentAttackPhase >= 2) {
		// 回転時ハルバードに当たり判定を出す.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		attackTempCollider_->SetSize({ kBasicHalberdColliderSize.x + 2.0f,kBasicHalberdColliderSize.y,kBasicHalberdColliderSize.z });
		attackTempTransform_.translate.y = -1.8f;
		CollisionManager::GetInstance()->AddColliderList(attackTempCollider_.get());
		attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	}

	attackTempCollider_->SetTransform(attackTempTransform_);
	attackTempCollider_->DrawCollider();
}

void Boss::PowerSlasherInitialize() {
	kMaxAttackTimer = kPowerSlasherStartGapTimerMax;
	attackTempTransform_.translate = { 1.0f,0.0f,0.0f };
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);
	attackTempCollider_->SetColliderType(ColliderType::kBox);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy));
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	halberdTransform_.SetParent(&transform_);
	powerSlasherHalberdCenter_.Initialize();
	powerSlasherHalberdCenter_.SetParent(&transform_);
	halberdTransform_.SetParent(&powerSlasherHalberdCenter_);
	attackTempTransform_.SetParent(&powerSlasherHalberdCenter_);
	attackTempCollider_->SetDamage(25.0f);
	attackTempCollider_->SetDamageCoolTime(3.0f);
}

void Boss::PowerSlasherUpdate() {
	Vector3 lenght;
	attackTempTransform_ = halberdTransform_;
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);

	Vector3 move = { 0.0f,0.0f,-kPowerSlasherSpeed };

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix({ 0.0f,transform_.rotate.y,0.0f });

	move = rotateMatrix.TransformNomal(move);

	switch (currentAttackPhase) {
	case 0: // ハルバードを構える.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kPowerSlasherHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kPowerSlasherHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(0.0f, kPowerSlasherModelStartRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPowerSlasherStayTimerMax);
		}
		break;
	case 1: // 前隙.
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			lenght = targetTransform_->translate - transform_.translate;
			// プレイヤーの位置によって攻撃が変わる.
			if (lenght.Length() < kPowerSlasherNearSlashRadius) {
				// 敵に近い位置なら2に遷移.
				NextAttackPhase(kPowerSlasherDashToSlashTimerMax);
			} else {
				// 敵から離れた位置なら3に遷移.
				currentAttackPhase++;
				NextAttackPhase(kPowerSlasherDashTimerMax);
			}
		}
		break;
	case 2: // 攻撃しながら構えなおす.
		destinationHalberdTransform_.translate = Easing(kPowerSlasherHalberdStartPos, kPowerSlasherHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kPowerSlasherHalberdStartRotate, kPowerSlasherHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		powerSlasherHalberdCenter_.rotate.y = Easing(0.0f, Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kPowerSlasherModelStartRotateY, kPowerSlasherModelFinishedRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			// 6(後隙)に遷移.
			currentAttackPhase++;
			currentAttackPhase++;
			currentAttackPhase++;
			NextAttackPhase(kPowerSlasherStayTimerMax);
		}
		break;
	case 3:  // 突進をする.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		lenght = targetTransform_->translate - transform_.translate;
		// プレイヤーの位置によって攻撃の終わるタイミングが変わる.
		if (lenght.Length() < kPowerSlasherSlashRadius) {
			// 射程圏内に入ったら4に遷移.
			NextAttackPhase(kPowerSlasherDashToSlashTimerMax);
		}

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			// 突進時間が終了したら4に遷移.
			NextAttackPhase(kPowerSlasherDashToSlashTimerMax);
		}
		break;
	case 4: // 突進しながら構えなおす.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		destinationHalberdTransform_.translate = Easing(kPowerSlasherHalberdStartPos, kPowerSlasherHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kPowerSlasherHalberdStartRotate, kPowerSlasherHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPowerSlasherDashToSlashTimerMax);
		}
		break;
	case 5: // 攻撃を行う.
		powerSlasherHalberdCenter_.rotate.y = Easing(0.0f, Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kPowerSlasherModelStartRotateY, kPowerSlasherModelFinishedRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPowerSlasherSlashStayTimerMax);
		}
		break;
	case 6: // 後隙(2または5から遷移される).
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPowerSlasherFinishedGapTimerMax);
		}
		break;
	case 7: // 見た目を戻す.
		powerSlasherHalberdCenter_.rotate.y = Easing(Radian(180.0f), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kPowerSlasherModelFinishedRotateY, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.translate = Easing(kPowerSlasherHalberdAttackPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kPowerSlasherHalberdAttackRotate, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			halberdTransform_.SetParent(&transform_);
		}
		break;
	}

	if (currentAttackPhase <= 5 && currentAttackPhase >= 2) {
		// 突進時や攻撃時にはハルバードに当たり判定を作る.
		attackTempCollider_->SetSize({ kBasicHalberdColliderSize.x + 2.0f,kBasicHalberdColliderSize.y,kBasicHalberdColliderSize.z });
		attackTempTransform_.translate.y = -1.8f;
		CollisionManager::GetInstance()->AddColliderList(attackTempCollider_.get());
		attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	}

	attackTempCollider_->SetTransform(attackTempTransform_);
	attackTempCollider_->DrawCollider();
}

void Boss::FangAttackInitialize() {
	//AttackFinished();

	kMaxAttackTimer = kFangAttackStartGapTimerMax;
}

void Boss::FangAttackUpdate() {
	switch (currentAttackPhase) {
	case 0: // 上昇しながらハルバードを前に構える.
		transform_.translate.y = Easing(kBasicPositionY, kFangAttackAnimPositionY / 2.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kFangAttackHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kFangAttackHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kFangAttackStartGapTimerMax);
			halberdTransform_.SetParent(&modelTransform_);
		}
		break;
	case 1: // 残りの上昇.
		transform_.translate.y = Easing(kFangAttackAnimPositionY / 2.0f, kFangAttackAnimPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.x = Easing(kFangAttackHalberdStartRotate.x, preTransform_.rotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kFangAttackHalberdPos, kFangAttackHalberdSpinPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kFangAttackSpinTimerMax);
			preTransform_ = transform_;
		}
		break;
	case 2: // その場で回転.
		transform_.rotate.x = Easing(preTransform_.rotate.x, preTransform_.rotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kFangAttackAttackTimerMax);
		}
		break;
	case 3: // 攻撃態勢に入りながら急降下.
		transform_.translate.y = Easing(kFangAttackAnimPositionY, kFangAttackAttackPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		transform_.rotate.x = Easing(preTransform_.rotate.x, kFangAttackAttackRotateX, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kFangAttackHalberdSpinPos, kFangAttackHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kFangAttackAttackGapTimerMax);
			float rotateY = transform_.rotate.y - Radian(90.0f);
			//float lenght = Vector3(targetTransform_->translate - transform_.translate).Length();
			//Transform newTransform = halberdTransform_;
			//std::vector<Vector3> spikePos_;
			//bool isShot_;
			//if (lenght <= (kFangAttackRadius / 3.0f) * 2.0f) {
			//	for (uint32_t i = 0; i < kFangAttackRadiusNum; i++) {
			//		newTransform = halberdTransform_;
			//		newTransform.translate = newTransform.translate + Random::GetInstance()->RandomCircleVector3({ kFangAttackRadius ,kFangAttackRadius ,kFangAttackRadius });
			//
			//		ProjectileManager::GetInstance()->CreateSpike(newTransform, 0, kCollisionEnemyAttack, 15.0f, 3.0f);
			//		spikePos_.push_back(newTransform.GetWorldPosition());
			//	}
			//} else {
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, Vector3(-RadianToVector(rotateY).x, 0.0f, RadianToVector(rotateY).y) * 20.0f, BulletType::kSpike, kCollisionEnemyAttack, 15.0f, 3.0f);
			//}

		}
		break;
	case 4: // 攻撃後の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kFangAttackFinishedGapTimerMax);
		}
		break;
	case 5: // 見た目を戻す.
		transform_.translate.y = Easing(kFangAttackAttackPositionY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		transform_.rotate.x = Easing(kFangAttackAttackRotateX, preTransform_.rotate.x, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.translate = Easing(kFangAttackHalberdAttackPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			halberdTransform_.SetParent(&transform_);
		}
		break;
	}
}

void Boss::NearAttackInitialize() {
	kMaxAttackTimer = kNearFirstStartGapTimerMax;
	randomYFlip = 1.0f;
	nearAttackSecondProbability_ = Easing(100.0f, 0.0f, currentHP_, maxHP_, EaseType::kConstant);
	nearAttackThirdProbability_ = Easing(50.0f, 0.0f, currentHP_, maxHP_, EaseType::kConstant);
	attackTempTransform_.Initialize();
	attackTempTransform_.translate = { 0.0f,0.0f,0.0f };
	attackTempTransform_.SetParent(&halberdTransform_);
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);
	attackTempCollider_->SetColliderType(ColliderType::kBox);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy));
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	attackTempCollider_->SetDamage(10.0f);
	attackTempCollider_->SetDamageCoolTime(0.4f);
	attackTempCollider_->SetActive(true);
}

void Boss::NearAttackUpdate() {
	attackTempCollider_->SetDamage(10.0f);
	attackTempCollider_->SetTransform(attackTempTransform_);
	switch (currentAttackPhase) {
	case -2:
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearAttackFinishedGapTimerMax);
		}

		break;
	case -1:
		destinationHalberdTransform_.translate = Easing(nearAttackHalPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(nearAttackHalRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(nearAttackModelRotateY, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	case 0: // ハルバードを構える.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kNearFirstHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kNearFirstHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(0.0f, kNearFirstModelStartRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearFirstStayTimerMax);
		}
		break;
	case 1: // 前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearFirstAttackTimerMax);
		}
		break;
	case 2: // 攻撃.
		destinationHalberdTransform_.translate = Easing(kNearFirstHalberdStartPos, kNearFirstHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kNearFirstHalberdStartRotate, kNearFirstHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kNearFirstModelStartRotateY, kNearFirstModelAttackRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
		CollisionManager::GetInstance()->AddColliderList(attackTempCollider_.get());

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			if (Random::GetInstance()->Probability(nearAttackSecondProbability_)) {
				NextAttackPhase(kNearFirstAttackGapTimerMax);
			} else {
				NextAttackPhase(kNearAttackGapTimerMax);
				currentAttackPhase = -2;
				nearAttackHalPos = destinationHalberdTransform_.translate;
				nearAttackHalRotate = destinationHalberdTransform_.rotate;
				nearAttackModelRotateY = modelTransform_.rotate.y;
			}
		}
		break;
	case 3: // 攻撃隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearSecondStartGapTimerMax);
			preTransform_ = transform_;
			nearAttackPreTransform_ = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		}
		break;
	case 4: // 2段目構え.
		destinationHalberdTransform_.translate = Easing(kNearFirstHalberdAttackPos, kNearSecondHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kNearFirstHalberdAttackRotate, kNearSecondHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = Easing(preTransform_.rotate.y, nearAttackPreTransform_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearSecondStayTimerMax);
		}
		break;
	case 5: // 2段目前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearSecondAttackTimerMax);
		}
		break;
	case 6: // 2段目攻撃.
		destinationHalberdTransform_.translate = Easing(kNearFirstHalberdAttackPos, kNearSecondHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kNearSecondHalberdStartRotate, kNearSecondHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kNearFirstModelAttackRotateY, kNearSecondModelAttackRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
		CollisionManager::GetInstance()->AddColliderList(attackTempCollider_.get());

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			if (Random::GetInstance()->Probability(nearAttackThirdProbability_)) {
				NextAttackPhase(kNearSecondAttackGapTimerMax);
			} else {
				NextAttackPhase(kNearAttackGapTimerMax);
				currentAttackPhase = -2;
				nearAttackHalPos = destinationHalberdTransform_.translate;
				nearAttackHalRotate = destinationHalberdTransform_.rotate;
				nearAttackModelRotateY = modelTransform_.rotate.y;
			}
		}
		break;
	case 7: // 攻撃隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearSecondStartGapTimerMax);
			halberdTransform_.SetParent(&modelTransform_);
			preTransform_ = transform_;
			nearAttackPreTransform_ = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		}

		break;
	case 8: // 3段目構え.
		destinationHalberdTransform_.translate = Easing(kNearSecondHalberdAttackPos, kNearThirdHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kNearSecondHalberdAttackRotate, kNearThirdHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kNearSecondModelAttackRotateY, kNearThirdModelStartRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.translate.y = Easing(kBasicPositionY, kNearThirdStartPosisionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = Easing(preTransform_.rotate.y, nearAttackPreTransform_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearThirdStayTimerMax);
		}
		break;
	case 9: // 3段目前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearThirdAttackTimerMax);
		}
		break;
	case 10: // 3段目攻撃.
		transform_.translate.y = Easing(kNearThirdStartPosisionY, kNearThirdAttackPosisionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		modelTransform_.rotate.x = Easing(0.0f, kNearThirdModelAttackRotateX, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		attackTempCollider_->SetDamage(30.0f);
		attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
		CollisionManager::GetInstance()->AddColliderList(attackTempCollider_.get());

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearThirdAttackGapTimerMax);
			ProjectileManager::GetInstance()->CreateWave(transform_, 25.0f, 1.0f, 1.0f, kCollisionEnemyAttack, 20.0f, 3.0f);
			Transform newTransform = halberdTransform_;
			for (uint32_t i = 0; i < kNearThirdAttackRadiusNum; i++) {
				newTransform = halberdTransform_;
				newTransform.translate += newTransform.GetWorldPosition() + Random::GetInstance()->RandomCircleVector3({ kNearThirdAttackRadius ,kNearThirdAttackRadius ,kNearThirdAttackRadius });
				ProjectileManager::GetInstance()->CreateSpike(newTransform, 0, kCollisionEnemyAttack, 20.0f, 3.0f);
			}
		}
		break;
	case 11: // 3段目後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearAttackFinishedGapTimerMax);
		}
		break;
	case 12:
		destinationHalberdTransform_.translate = Easing(kNearThirdHalberdStartPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kNearThirdHalberdStartRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		modelTransform_.rotate.x = Easing(kNearThirdModelAttackRotateX, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		transform_.translate.y = Easing(kNearThirdAttackPosisionY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
	attackTempCollider_->DrawCollider();

}
