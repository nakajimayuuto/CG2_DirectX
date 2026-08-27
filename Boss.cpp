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
		&Boss::Phase2BounsInitialize,
		&Boss::Phase2DiffusionBulletInitialize,
		&Boss::Phase2MovingBulletInitialize,
		&Boss::Phase2WaveInitialize,
		&Boss::Phase2SpinningInitialize,
		&Boss::Phase2PowerSlasherInitialize,
		&Boss::Phase2FangAttackInitialize,
		&Boss::DownInitialize,
		&Boss::SuperDownInitialize,
		&Boss::LastDownInitialize,
		&Boss::SpecialAttackInitialize,
		&Boss::ThreeWayWaveInitialize,
		&Boss::AutoHalberdInitialize,
		&Boss::InfinitySlasherInitialize,
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
		&Boss::Phase2BounsUpdate,
		&Boss::Phase2DiffusionBulletUpdate,
		&Boss::Phase2MovingBulletUpdate,
		&Boss::Phase2WaveUpdate,
		&Boss::Phase2SpinningUpdate,
		&Boss::Phase2PowerSlasherUpdate,
		&Boss::Phase2FangAttackUpdate,
		&Boss::DownUpdate,
		&Boss::SuperDownUpdate,
		&Boss::LastDownUpdate,
		&Boss::SpecialAttackUpdate,
		&Boss::ThreeWayWaveUpdate,
		&Boss::AutoHalberdUpdate,
		&Boss::InfinitySlasherUpdate,
};

Boss::~Boss() {
	delete targetTransform_;
}

void Boss::Initialize() {
	model_.Initialize("boss");
	model_.SetBlendMode(BlendMode::kNormalCullNone);
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
	transform_.Initialize();
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
	colliderRadius_ = 10.0f;
	colliderSize_ = kBasicColliderSize;
	collisionAttribute_ = CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemy);
	collisionMask_ =
		CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack) |
		CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer);

	colliderColor_ = { 0.6f,0.3f,1.0f,1.0f };

	attackTempTransform_.Initialize();
	attackTempCollider_ = std::make_unique<Collider>();
	attackTempTransform_.SetParent(&transform_);
	attackTempCollider_->SetRadius(5.0f);
	attackTempCollider_->SetColliderType(ColliderType::kSphere);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack));
	attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });

	switch (DifficultyManager::GetInstance()->GetCurrentDifficulty()) {
	case kDifficultyEasy:
		maxHP_ = 1500.0f;
		bossName_ = "name_easy";
		bossNameMirror_ = "name_easy_mirror";
		break;
	case kDifficultyNormal:
		maxHP_ = 3000.0f;
		bossName_ = "name_normal";
		bossNameMirror_ = "name_normal_mirror";
		break;
	case kDifficultyHard:
		maxHP_ = 10000.0f;
		bossName_ = "name_hard";
		bossNameMirror_ = "name_hard_mirror";
		break;
	case kDifficultyHell:
		maxHP_ = 100000.0f;
		bossName_ = "name_hard";
		bossNameMirror_ = "name_hard_mirror";
		break;
	default:
		maxHP_ = 3000.0f;
		bossName_ = "name_normal";
		bossNameMirror_ = "name_normal_mirror";
		break;
	}
	currentHP_ = maxHP_;

	hpGauge = std::make_unique<HPGauge>();
	hpGauge->Initialize(&currentHP_, maxHP_, { 800.0f,30.0f });
	hpGauge->SetPosition({ 0.0f,-270.0f });
	hpGauge->SetScale({ 1.0f, 1.0f });
	bossNameScaleX_ = 1.0f;

	DifficultyManager::GetInstance()->SetBossHPData(&currentHP_, maxHP_);

	GameCamera::GetInstance()->SetEnemyTransform(&transform_);

	halberdLeft_ = std::make_unique<MirrorHalberd>();
	halberdLeft_->Initialize(MirrorHalberd::HalberdName::kLeft);
	halberdLeft_->SetParent(&transform_);
	halberdLeft_->SetBossTransform(&transform_);

	halberdRight_ = std::make_unique<MirrorHalberd>();
	halberdRight_->Initialize(MirrorHalberd::HalberdName::kRight);
	halberdRight_->SetParent(&transform_);
	halberdRight_->SetBossTransform(&transform_);

	halberdLeftAttackTimer_ = 0.0f;
	halberdRightAttackTimer_ = kHalberdFarAttackTimerMax_ / 2.0f;


	if (gGamePhase == GamePhase::kBossPhase2 || gGamePhase == GamePhase::kBossPhaseChangeAnim) {
		phase_ = Phase::kPhase2;

		currentHP_ = maxHP_ / 2.0f;
		GameCamera::GetInstance()->SetRotate({ 0.0f,0.0f,0.0f });
		halberdRight_->SetPosition(kBasicHalberdRightPos);
		halberdRight_->SetRotate({ 0.0f,0.0f,0.0f });
		halberdLeft_->SetPosition(kBasicHalberdLeftPos);
		halberdLeft_->SetRotate({ 0.0f,0.0f,0.0f });
		halberdRight_->SetIsActive(true);
		halberdLeft_->SetIsActive(true);
		halberdRight_->SetActive(false);
		halberdLeft_->SetActive(false);
		halberdLeft_->SetTargetTransform(targetTransform_);
		halberdRight_->SetTargetTransform(targetTransform_);
		Phase2Initialize();
	} else {
		phase_ = Phase::kPhase1;
		Phase1Initialize();
	}

	LightManager::GetInstance()->CreatePointLight("boss_light");
	LightManager::GetInstance()->GetLightData("boss_light")->color = { 0.5f,0.5f,1.0f,1.0f };
	LightManager::GetInstance()->GetLightData("boss_light")->radius = 5.0f;
	LightManager::GetInstance()->GetLightData("boss_light")->intensity = 1.0f;

	emitter_ = std::make_unique<Emitter>();
	emitter_->SetParticle(ParticleManager::GetInstance()->GetParticles("blue_fire"));
	emitter_->Initialize(Transform::GetInitialValue(kBasicColliderSize, { 0.0f,0.0f,0.0f }, transform_.GetWorldPosition()), 1, 0.1f);

	downEmitter_ = std::make_unique<Emitter>();
	downEmitter_->SetParticle(ParticleManager::GetInstance()->GetParticles("star"));
	downEmitter_->Initialize(Transform::GetInitialValue({ kBasicColliderSize.x,0.5f,kBasicColliderSize.z }, { 0.0f,0.0f,0.0f }, transform_.GetWorldPosition()), 1, 0.1f);

	if (gGamePhase == GamePhase::kGameStartAnim) {
		StartAnimInitialize();
	} else if (gGamePhase == GamePhase::kBossPhaseChangeAnim) {
		PhaseChangeAnimInitialize();
	} else if (gGamePhase == GamePhase::kBossLastJaronaAnim) {
		DeathAnimInitialize();
	}

	if (DifficultyManager::GetInstance()->GetCurrentDifficulty() == Difficulty::kDifficultyEasy) {
		useSpecialAttack_ = false;
	} else {
		useSpecialAttack_ = true;
	}

	damageAmountRecord_ = 0.0f;

	isChangePhase_ = false;
	isImmune_ = false;
	debugHpScale_ = 1.0f;

	isDeath_ = false;
#ifdef _DEBUG
	useDebugUpdateStop = true;
#endif // _DEBUG

	isDownThreeWayShot_ = false;
	isDownAutoHalberd_ = false;
	isDownInfinitySlasher_ = false;

	downSpecialAttackCount_ = 0;

	downDamageMangification_ = 1.0f;
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
	case Boss::DistanceName::kAutoHalberd:
		autoHalberdAttackDatas_.push_back(newData);
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
	SetAttackData(Attacks::kWaveShot, 0.5f, DistanceName::kFar);

	SetAttackData(Attacks::kBounsShot, 0.5f, DistanceName::kMiddle);
	SetAttackData(Attacks::kFangAttack, 0.5f, DistanceName::kMiddle);
	if (DifficultyManager::GetInstance()->GetCurrentDifficulty() != Difficulty::kDifficultyEasy) {
		SetAttackData(Attacks::kPowerSlasher, 0.3f, DistanceName::kMiddle);
		SetAttackData(Attacks::kSpinningHalberd, 0.5f, DistanceName::kMiddle);
	}
	SetAttackData(Attacks::kWaveShot, 0.6f, DistanceName::kMiddle);

	SetAttackData(Attacks::kNearAttack, 0.7f, DistanceName::kNear);
	SetAttackData(Attacks::kMovingShot, 0.3f, DistanceName::kNear);
	if (DifficultyManager::GetInstance()->GetCurrentDifficulty() == Difficulty::kDifficultyHard) {
		SetAttackData(Attacks::kPowerSlasher, 0.1f, DistanceName::kNear);
	}

}

void Boss::Phase2Initialize() {
	nearAttackDatas_.clear();
	middleAttackDatas_.clear();
	farAttackDatas_.clear();
	autoHalberdAttackDatas_.clear();


	switch (DifficultyManager::GetInstance()->GetCurrentDifficulty()){
	case Difficulty::kDifficultyEasy:
		SetAttackData(Attacks::kBulletShot, 1.0f, DistanceName::kFar);
		SetAttackData(Attacks::kPhase2MovingShot, 0.5f, DistanceName::kFar);
		SetAttackData(Attacks::kDiffusionShot, 0.5f, DistanceName::kFar);
		SetAttackData(Attacks::kWaveShot, 0.5f, DistanceName::kFar);


		SetAttackData(Attacks::kBounsShot, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kFangAttack, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kPowerSlasher, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kSpinningHalberd, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kWaveShot, 0.5f, DistanceName::kMiddle);

		SetAttackData(Attacks::kNearAttack, 0.7f, DistanceName::kNear);
		SetAttackData(Attacks::kMovingShot, 0.3f, DistanceName::kNear);
		break;
	case Difficulty::kDifficultyNormal:
		SetAttackData(Attacks::kBulletShot, 1.0f, DistanceName::kFar);
		SetAttackData(Attacks::kPhase2MovingShot, 0.5f, DistanceName::kFar);
		SetAttackData(Attacks::kPhase2DiffusionShot, 0.5f, DistanceName::kFar);
		SetAttackData(Attacks::kWaveShot, 0.5f, DistanceName::kFar);

		SetAttackData(Attacks::kPhase2BounsShot, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kPhase2FangAttack, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kPhase2PowerSlasher, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kSpinningHalberd, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kWaveShot, 0.5f, DistanceName::kMiddle);

		SetAttackData(Attacks::kNearAttack, 0.7f, DistanceName::kNear);
		SetAttackData(Attacks::kPhase2MovingShot, 0.3f, DistanceName::kNear);
		SetAttackData(Attacks::kPowerSlasher, 0.1f, DistanceName::kNear);
		break;
	case Difficulty::kDifficultyHard:
		SetAttackData(Attacks::kBulletShot, 1.0f, DistanceName::kFar);
		SetAttackData(Attacks::kPhase2MovingShot, 0.5f, DistanceName::kFar);
		SetAttackData(Attacks::kPhase2DiffusionShot, 0.5f, DistanceName::kFar);
		SetAttackData(Attacks::kPhase2WaveShot, 0.5f, DistanceName::kFar);

		SetAttackData(Attacks::kPhase2BounsShot, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kPhase2FangAttack, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kPhase2PowerSlasher, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kPhase2SpinningHalberd, 0.5f, DistanceName::kMiddle);
		SetAttackData(Attacks::kPhase2WaveShot, 0.5f, DistanceName::kMiddle);

		SetAttackData(Attacks::kNearAttack, 0.7f, DistanceName::kNear);
		SetAttackData(Attacks::kPhase2MovingShot, 0.3f, DistanceName::kNear);
		SetAttackData(Attacks::kPowerSlasher, 0.1f, DistanceName::kNear);
		break;
	}
	

	SetAttackData(Attacks::kBulletShot, 1.0f, DistanceName::kAutoHalberd);
	SetAttackData(Attacks::kDiffusionShot, 0.5f, DistanceName::kAutoHalberd);
	SetAttackData(Attacks::kPhase2MovingShot, 0.5f, DistanceName::kAutoHalberd);
	SetAttackData(Attacks::kWaveShot, 0.5f, DistanceName::kAutoHalberd);

}

void Boss::Update() {
	dopamineSpeed_ = DifficultyManager::GetInstance()->GetDopamineSpeed();

	difficultyMagnificationTime = DifficultyManager::GetInstance()->GetSpeedMagnification();

	deltaTime_ = DeltaTime::GetInstance()->GetGameTime();

	halberdLeft_->SetGameTime(deltaTime_ * difficultyMagnificationTime * dopamineSpeed_);
	halberdRight_->SetGameTime(deltaTime_ * difficultyMagnificationTime * dopamineSpeed_);

	if (gGamePhase == GamePhase::kGameStartAnim) {
		StartAnimationUpdate();

		EffectUpdate();
		return;
	} else if (gGamePhase == GamePhase::kBossPhaseChangeAnim) {
		PhaseChangeAnimationUpdate();

		EffectUpdate();
		return;
	} else if (gGamePhase == GamePhase::kBossLastJaronaAnim) {
		DeathAnimationUpdate();

		EffectUpdate();
		return;
	}

	DistanceCheckUpdate();

	HalberdStanceUpdate();

	downSpecialAttackCount_ = 0;
	if (isDownThreeWayShot_) {
		downSpecialAttackCount_++;
	}
	if (isDownAutoHalberd_) {
		downSpecialAttackCount_++;
	}
	if (isDownInfinitySlasher_) {
		downSpecialAttackCount_++;
	}

	switch (downSpecialAttackCount_) {
	case 0:
		downDamageMangification_ = 1.0f;
		break;
	case 1:
		downDamageMangification_ = 1.25f;
		break;
	case 2:
		downDamageMangification_ = 1.5f;
		if (DifficultyManager::GetInstance()->GetCurrentDifficulty() == Difficulty::kDifficultyNormal) {
			useSpecialAttack_ = false;
		}
		break;
	case 3:
		downDamageMangification_ = 2.0f;
		useSpecialAttack_ = false;
		break;
	default:
		downDamageMangification_ = 1.0f;
		break;
	}

	halberdLeft_->SetTargetIsAttact(isAttack_);
	halberdRight_->SetTargetIsAttact(isAttack_);
	if (!isAttack_) {
		damageCountFirst_ = 0;
		damageCountSecond_ = 0;
		damageCountThird_ = 0;
	}

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
	ImGui::Text("%s", useDebugUpdateStop ? "true" : "false");


	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("scale", reinterpret_cast<float*>(&transform_.scale), 0.05f, 0.0f, 5.0f);
	ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&imRotate), 1.0f, -360.0f, 360.0f);
	ImGui::DragFloat3("translate", reinterpret_cast<float*>(&transform_.translate), 0.25f, -100.0f, 100.0f);
	ImGui::Text("");
	ImGui::DragFloat("HP", &currentHP_, 1.0f, 0.0f, maxHP_);

	if (ImGui::Button("currentHPChangeMaxHP")) {
		maxHP_ = currentHP_;
		hpGauge->Initialize(&currentHP_, maxHP_, { 800.0f,60.0f });
		hpGauge->SetPosition({ 0.0f,-300.0f });
	}
	ImGui::Text("");
	if (isPlayAttack_) {
		ImGui::Text("currentAttack : %s", magic_enum::enum_name(currentAttack_).data());
	} else {
		ImGui::Text("currentAttack : None");
	}
	if (ImGui::Button("AttackFinnish")) {
		AttackFinished();
	}
	ImGui::Text("");
	for (Attacks attack : magic_enum::enum_values<Attacks>()) {
		if (static_cast<size_t>(attack) == std::size(pUpdateFunc)) {
			break;
		}

		if (ImGui::Button(magic_enum::enum_name(attack).data())) {
			attackRequest_ = attack;
		}
	}

	transform_.rotate = Radian(imRotate);

	//ImGui::SliderFloat("gaugeScale", &debugHpScale_, 0.0f, 1.0f);
	//hpGauge->SetScale({ debugHpScale_, 1.0f });

	ImGui::End();
#endif // _DEBUG

	if (damageCoolTimer_ > 0.0f) {
		damageCoolTimer_ -= deltaTime_;
		if (damageCoolTimer_ <= 0.0f) {
			damageCoolTimer_ = 0.0f;
		}
	}

	RootUpdate();

	AttackInitialize();

	AttackUpdate();

	halberdTransform_.scale = Lerp(halberdTransform_.scale, destinationHalberdTransform_.scale, kDestinationCompletionRate * DeltaTime::GetInstance()->GetGameTimePerFrame());
	halberdTransform_.rotate = LerpShortAngle(halberdTransform_.rotate, destinationHalberdTransform_.rotate, kDestinationCompletionRate * DeltaTime::GetInstance()->GetGameTimePerFrame());
	halberdTransform_.translate = Lerp(halberdTransform_.translate, destinationHalberdTransform_.translate, kDestinationCompletionRate * DeltaTime::GetInstance()->GetGameTimePerFrame());

	CollisionManager::GetInstance()->AddColliderList(this);
	EffectUpdate();
}

void Boss::EffectUpdate() {

	emitter_->SetTransform(Transform::GetInitialValue((kBasicColliderSize * 3.0f), { 0.0f,0.0f,0.0f }, transform_.GetWorldPosition()));
	emitter_->Update();
	LightManager::GetInstance()->SetLightPos("boss_light", transform_.GetWorldPosition());
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
	transform_.rotate.y = LerpShortAngle(transform_.rotate.y, destinationAngleY_, 0.25f * DeltaTime::GetInstance()->GetGameTimePerFrame());

	destinationHalberdTransform_.rotate = basicHalberdRotate;
	destinationHalberdTransform_.translate = basicHalberdPos;

	if (attackCoolTimer_ >= attackCoolTimeMax_) {
		attackCoolTimeMax_ = 2.0f;
		attackCoolTimer_ = 0.0f;

		if (gGamePhase == GamePhase::kBossPhase2) {
			if (useSpecialAttack_) {
				if (Random::GetInstance()->Probability(Easing(0.0f, 100.0f, damageAmountRecord_, maxHP_ / 4.0f, EaseType::kConstant))) {
					attackRequest_ = Attacks::kSpecialAttack;
				} else {
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
			} else {
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
		} else {
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

	halberdRight_->Update();
	halberdLeft_->Update();

	switch (currentDistance_) {
	case Boss::DistanceName::kNear:
		basicHalberdPos = kBasicHalberdNearPos;
		basicHalberdRotate = kBasicHalberdNearRotate;

		basicHalberdLeftRotate.x = kBasicHalberdNormalLeftRotateX;
		basicHalberdRightRotate.x = kBasicHalberdNormalRightRotateX;
		halberdLeftAttackTimer_ = 0.0f;
		halberdRightAttackTimer_ = kHalberdFarAttackTimerMax_ / 2.0f;
		break;
	case Boss::DistanceName::kMiddle:
		basicHalberdPos = kBasicHalberdMiddlePos;
		basicHalberdRotate = kBasicHalberdMiddleRotate;

		basicHalberdLeftRotate.x = kBasicHalberdNormalLeftRotateX;
		basicHalberdRightRotate.x = kBasicHalberdNormalRightRotateX;
		halberdLeftAttackTimer_ = 0.0f;
		halberdRightAttackTimer_ = kHalberdFarAttackTimerMax_ / 2.0f;
		break;
	case Boss::DistanceName::kFar:
		basicHalberdPos = kBasicHalberdFarPos;
		basicHalberdRotate = kBasicHalberdFarRotate;

		basicHalberdLeftRotate.x = kBasicHalberdFarLeftRotateX;
		basicHalberdRightRotate.x = kBasicHalberdFarRightRotateX;
		break;
	}

	if (isPlayAttack_) {
		return;
	}

	halberdLeft_->SetRotateX(LerpShortAngle(halberdLeft_->GetRotate().x, basicHalberdLeftRotate.x, kDestinationCompletionRate * DeltaTime::GetInstance()->GetGameTimePerFrame()));
	halberdRight_->SetRotateX(LerpShortAngle(halberdRight_->GetRotate().x, basicHalberdRightRotate.x, kDestinationCompletionRate * DeltaTime::GetInstance()->GetGameTimePerFrame()));

	if (phase_ == Phase::kPhase2) {
		if (currentDistance_ == Boss::DistanceName::kFar) {
			Phase2HalberdFarAttackUpdate();
		}
	}
}

void Boss::Phase2HalberdFarAttackUpdate() {
	if (useDebugUpdateStop) {
		return;
	}
	Vector3 direction;
	halberdLeftAttackTimer_ += deltaTime_;
	halberdRightAttackTimer_ += deltaTime_;

	if (halberdLeftAttackTimer_ >= kHalberdFarAttackTimerMax_) {
		direction = (targetTransform_->translate - halberdLeft_->GetTransform().GetWorldPosition()).Normalize();
		if (Random::GetInstance()->Probability(33.0f)) {
			ProjectileManager::GetInstance()->CreateBullet(halberdLeft_->GetTransform(), direction * 10.0f, BulletType::kBounce, kCollisionEnemyAttack, 10.0f, 3.0f);
		} else {
			ProjectileManager::GetInstance()->CreateBullet(halberdLeft_->GetTransform(), direction * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f);
		}

		halberdLeftAttackTimer_ = 0.0f;
	}

	if (halberdRightAttackTimer_ >= kHalberdFarAttackTimerMax_) {
		direction = (targetTransform_->translate - halberdRight_->GetTransform().GetWorldPosition()).Normalize();
		if (Random::GetInstance()->Probability(33.0f)) {
			ProjectileManager::GetInstance()->CreateBullet(halberdRight_->GetTransform(), direction * 10.0f, BulletType::kBounce, kCollisionEnemyAttack, 10.0f, 3.0f);
		} else {
			ProjectileManager::GetInstance()->CreateBullet(halberdRight_->GetTransform(), direction * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f);
		}
		halberdRightAttackTimer_ = 0.0f;
	}
}

void Boss::Draw() {
	Renderer* renderer = Renderer::GetInstance();
	//model_.Draw(modelTransform_,true);
	renderer->DrawModel(modelTransform_, &model_, true);
	renderer->DrawShadow(modelTransform_, &model_, { 0.0f,0.0f,0.0f,1.0f });

	renderer->DrawModel(halberdTransform_, &halberdModel_, true);
	renderer->DrawShadow(halberdTransform_, &halberdModel_, { 0.0f,0.0f,0.0f,1.0f });

	halberdRight_->Draw();
	halberdLeft_->Draw();
#ifdef _DEBUG

	for (Vector3& pos : anchorPoints_) {
		renderer->DrawSphereWireFrame(Transform::GetInitialValue({ 0.1f,0.1f,0.1f }, { 0.0f,0.0f,0.0f }, pos), { 0.5f,0.5f,1.0f,1.0f });
	}

#endif // _DEBUG

	DrawCollider();

	if (gGamePhase != GamePhase::kTutorial && gGamePhase != GamePhase::kBossLastJaronaAnim && gGamePhase != GamePhase::kGameClearStage) {
		hpGauge->Draw();
		if (gGamePhase == GamePhase::kBossPhase2 || gGamePhase == GamePhase::kBossPhaseChangeAnim) {
			renderer->DrawSprite(Transform::GetInitialValue(Vector3(bossNameScaleX_, 1.0f, 1.0f) * 0.5f, { 0.0f,0.0f,0.0f }, { 0.0f,-320.0f,0.0f }), bossNameMirror_, { 1.0f,1.0f,1.0f,1.0f });
		} else {
			renderer->DrawSprite(Transform::GetInitialValue(Vector3(bossNameScaleX_, 1.0f, 1.0f) * 0.5f, { 0.0f,0.0f,0.0f }, { 0.0f,-320.0f,0.0f }), bossName_, { 1.0f,1.0f,1.0f,1.0f });
		}
	}
}

void Boss::OnCollision(Collider* other) {
	float preHP = currentHP_;
	float tempMag = 1.0f;
	std::string sndName = "snd_boss_damage";
	Transform damageTransform = transform_;
	damageTransform.translate += Random::GetInstance()->RandomVector3(-Vector3( kBasicColliderSize.x /2.0f,0.5f,kBasicColliderSize.z / 2.0f ), Vector3(kBasicColliderSize.x / 2.0f, 0.5f, kBasicColliderSize.z / 2.0f));
	damageTransform.translate.y += 2.5f;
	if (isPlayAttack_) {
		if (currentAttack_ == Attacks::kAutoHalberd || currentAttack_ == Attacks::kInfinitySlasher) {
			tempMag = 0.0f;
			sndName = "snd_parry";
		}
	}

	if (isImmune_) {
		return;
	}

	if (other->GetCollisionAttribute() == CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer)) {
		return;
	}
	//if ((other->GetCollisionAttribute() & CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack)) == 0x0) {
	switch (other->GetDamageType()) {
	case 1:
		if (damageCountFirst_ <= 0) {
			currentHP_ -= other->GetDamage() * downDamageMangification_ * tempMag;

			DeltaTime::GetInstance()->SetHitStop(0.05f);
			damageCountFirst_++;
			SoundManager::GetInstance()->SoundPlay(sndName, 1.0f, 1.0f, kSoundEffect);
			InputManager::GetInstance()->SetVibration(0.1f, 0.1f, 0.25f);

			if (tempMag == 0.0f) {
				ParticleManager::GetInstance()->SpawnNumbers(0.0f, damageTransform, { 0.4f,0.4f,0.4f });
			} else {
				ParticleManager::GetInstance()->SpawnNumbers(other->GetDamage() * downDamageMangification_, damageTransform, { 0.5f,0.5f,1.0f });
			}
		}
		break;
	case 2:
		if (damageCountSecond_ <= 1) {
			if (damageCoolTimer_ <= 0.0f) {
				currentHP_ -= other->GetDamage() * downDamageMangification_ * tempMag;

				DeltaTime::GetInstance()->SetHitStop(0.05f);

				damageCoolTimer_ = other->GetDamageCoolTime();

				damageCountSecond_++;
				SoundManager::GetInstance()->SoundPlay(sndName, 1.0f, 1.0f, kSoundEffect);
				InputManager::GetInstance()->SetVibration(0.1f, 0.1f, 0.25f);

				if (tempMag == 0.0f) {
					ParticleManager::GetInstance()->SpawnNumbers(0.0f, damageTransform, { 0.4f,0.4f,0.4f });
				} else {
					ParticleManager::GetInstance()->SpawnNumbers(other->GetDamage() * downDamageMangification_, damageTransform, { 0.5f,0.5f,1.0f });
				}
			}
		}
		break;
	case 3:
		if (damageCountThird_ <= 4) {
			if (damageCoolTimer_ <= 0.0f) {
				currentHP_ -= other->GetDamage() * downDamageMangification_ * tempMag;
				DeltaTime::GetInstance()->SetHitStop(0.05f);

				damageCoolTimer_ = other->GetDamageCoolTime();
				damageCountThird_++;
				SoundManager::GetInstance()->SoundPlay(sndName, 1.0f, 1.0f, kSoundEffect);
				InputManager::GetInstance()->SetVibration(0.1f, 0.1f, 0.25f);

				if (tempMag == 0.0f) {
					ParticleManager::GetInstance()->SpawnNumbers(0.0f, damageTransform, { 0.4f,0.4f,0.4f });
				} else {
					ParticleManager::GetInstance()->SpawnNumbers(other->GetDamage() * downDamageMangification_, damageTransform, { 0.5f,0.5f,1.0f });
				}

				if (damageCountFirst_ >= 1) {
					if (damageCountSecond_ >= 2) {
						if (damageCountThird_ == 4) {
							if (currentAttack_ != Attacks::kDown
								&& currentAttack_ != Attacks::kWarp
								&& currentAttack_ != Attacks::kSuperDown
								&& currentAttack_ != Attacks::kSpecialAttack
								&& currentAttack_ != Attacks::kThreeWayWave
								&& currentAttack_ != Attacks::kAutoHalberd
								&& currentAttack_ != Attacks::kInfinitySlasher
								) {
								DeltaTime::GetInstance()->SetHitStop(0.2f);
								AttackFinished();
								attackRequest_ = Attacks::kDown;
								AttackInitialize();
							}
						}
					}
				}

				if (currentAttack_ == Attacks::kDown) {
					damageCountFirst_ = 0;
					damageCountSecond_ = 0;
				}
			}
		}
		break;
	default:
		if (damageCoolTimer_ <= 0.0f) {
			currentHP_ -= other->GetDamage() * downDamageMangification_ * tempMag;

			DeltaTime::GetInstance()->SetHitStop(0.05f);

			damageCoolTimer_ = other->GetDamageCoolTime();
			SoundManager::GetInstance()->SoundPlay(sndName, 1.0f, 1.0f, kSoundEffect);
			InputManager::GetInstance()->SetVibration(0.1f, 0.1f, 0.25f);

			if (tempMag == 0.0f) {
				ParticleManager::GetInstance()->SpawnNumbers(0.0f, damageTransform, { 0.4f,0.4f,0.4f });
			} else {
				ParticleManager::GetInstance()->SpawnNumbers(other->GetDamage() * downDamageMangification_, damageTransform, { 0.5f,0.5f,1.0f });
			}
		}
		break;
	}

	if (currentAttack_ == Attacks::kThreeWayWave) {
		if (isPlayAttack_) {
			DeltaTime::GetInstance()->SetHitStop(0.5f);
			AttackFinished();
			attackRequest_ = Attacks::kSuperDown;
			isDownThreeWayShot_ = true;
			AttackInitialize();
		}
	}

	if (phase_ == Phase::kPhase1) {
		if (currentHP_ < (maxHP_ / 2.0f)) {
			DeltaTime::GetInstance()->SetHitStop(0.5f);
			phase_ = Phase::kPhase2;
			isChangePhase_ = true;
			gGameProgress = GameProgress::kPhase2Clear;
			SoundManager::GetInstance()->SoundPause("mus_phase1_intro");
			SoundManager::GetInstance()->SoundPause("mus_phase1");
			InputManager::GetInstance()->SetVibration(0.5f, 0.5f, 0.5f);
		}
	}

	if (phase_ == Phase::kPhase2) {
		if (currentHP_ < 1.0f) {
			currentHP_ = 1.0f;
			AttackFinished();
			DeltaTime::GetInstance()->SetHitStop(0.5f);
			attackRequest_ = Attacks::kLastDown;
			phase_ = Phase::kPhase3;
			SoundManager::GetInstance()->SoundPause("mus_phase2_intro");
			SoundManager::GetInstance()->SoundPause("mus_phase2");
			InputManager::GetInstance()->SetVibration(0.5f, 0.5f, 0.5f);
			AttackInitialize();

		}
	}

	if (phase_ == Phase::kPhase3) {
		if (currentHP_ < 0.0f) {
			currentHP_ = 0.0f;
			DeltaTime::GetInstance()->SetHitStop(1.0f);
			DeltaTime::GetInstance()->SetGameTimeSpeed(0.2f);
			phase_ = Phase::kFinished;
			InputManager::GetInstance()->SetVibration(0.5f, 0.5f, 0.5f);
			isChangePhase_ = true;
		}
	}

	damageAmountRecord_ += preHP - currentHP_;
}

void Boss::SlashEffectCreate(Transform* targetTransform, uint32_t num) {
	Transform effectCreate;
	for (uint32_t i = 0; i < 3; i++) {
		effectCreate.Initialize();
		effectCreate.SetParent(targetTransform);
		effectCreate.translate = Random::GetInstance()->RandomVector3(-(*targetTransform).scale / 2.0f, (*targetTransform).scale / 2.0f);

		ParticleManager::GetInstance()->SpawnParticles("cross", effectCreate.GetWorldPosition());
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

	attackTempCollider_->SetRadius(4.0f);

	(this->*pInitializeFunc[static_cast<size_t>(currentAttack_)])();
}

void Boss::AttackUpdate() {
	if (!isPlayAttack_) {
		return;
	}

	(this->*pUpdateFunc[static_cast<size_t>(currentAttack_)])();

	currentAttackTimer_ += deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;

	if (currentAttack_ == Attacks::kAutoHalberd) {
		if (halberdLeft_->GetIsDeath() && halberdRight_->GetIsDeath()) {
			if (isPlayAttack_) {
				autoHalberdStop_ = true;
				isDownAutoHalberd_ = true;
				DeltaTime::GetInstance()->SetHitStop(0.5f);
				halberdLeft_->AutoAttackStop();
				halberdRight_->AutoAttackStop();
				AttackFinished();
				attackRequest_ = Attacks::kSuperDown;
				AttackInitialize();
			}
		}
	}
}

void Boss::AttackFinished() {
	if (currentAttack_ == Attacks::kAutoHalberd && !autoHalberdStop_) {
		isColliderActive_ = true;
		autoHalberdAttackPhase_ = 0;
		currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
		kMaxAttackTimer = kAutoHalberdStartGapTimerMax; // 攻撃のタイマー最大値.
		currentAttackPhase = 0; // 攻撃のフェーズ.
		SetCurrentDistanceHalberdTransform();
		return;
	}

	autoHalberdStop_ = false;
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

void Boss::DownInitialize() {
	SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
	kMaxAttackTimer = kDonwStartTimer;
	donwAnimHalberdVelocityY_ = 2.0f;
	downAnimHalberdRotate_ = halberdTransform_.rotate;
	downAnimHalberdPos_ = halberdTransform_.translate;
	halberdTransform_.SetParent(&transform_);
	modelTransform_.rotate = { 0.0f,0.0f,0.0f };
	modelTransform_.scale = { 1.0f,1.0f,1.0f };
	transform_.scale = { 1.0f,1.0f,1.0f };

	halberdLeft_->SetPosition(kBasicHalberdLeftPos);
	halberdLeft_->SetRotate(basicHalberdLeftRotate);
	halberdLeft_->SetParent(&transform_);
	halberdRight_->SetPosition(kBasicHalberdRightPos);
	halberdRight_->SetRotate(basicHalberdRightRotate);
	halberdRight_->SetParent(&transform_);
}

void Boss::DownUpdate() {
	Transform emitterTransform;
	switch (currentAttackPhase) {
	case 0:
		transform_.translate.y = Easing(kBasicPositionY, kDownStartPosY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.x = Easing(0.0f, kDownStayRotateX / 2.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		destinationHalberdTransform_.translate = Easing(downAnimHalberdPos_, kDownHalberdPos_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(downAnimHalberdRotate_, kDownHalberdRotate_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDonwStartTimer);
		}
		break;
	case 1:
		transform_.translate.y = Easing(kDownStartPosY, kDownStayPosY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		transform_.rotate.x = Easing(kDownStayRotateX / 2.0f, kDownStayRotateX, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDonwStayTimer);
		}
		break;
	case 2: // 攻撃の前隙.
		emitterTransform = transform_;
		emitterTransform.scale = { kBasicColliderSize.x * 3.0f,0.5f,kBasicColliderSize.z * 3.0f };
		emitterTransform.translate.y += colliderSize_.y;
		downEmitter_->SetTransform(emitterTransform);
		downEmitter_->Update();
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDonwFinsihTimer);
			downAnimHalberdPos_ = destinationHalberdTransform_.translate;
		}
		break;
	case 3:
		transform_.translate.y = Easing(kDownStayPosY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.translate = Easing(kDownHalberdPos_, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kDownHalberdRotate_, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		transform_.rotate.x = Easing(kDownStayRotateX, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			transform_.rotate.x = 0.0f;
			attackRequest_ = Attacks::kWarp;
			AttackInitialize();
		}
		break;
	}
	downEmitter_->DebugDraw();
}

void Boss::SuperDownInitialize() {
	hpGauge->SetColor({ 1.0f,1.0f,0.1f });
	hpGauge->SetBackColor({ 1.0f,0.1f,0.1f });
	SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
	InputManager::GetInstance()->SetVibration(0.3f, 0.3f, 0.5f);
	kMaxAttackTimer = kDonwStartTimer;
	donwAnimHalberdVelocityY_ = 2.0f;
	downAnimHalberdRotate_ = halberdTransform_.rotate;
	downAnimHalberdPos_ = halberdTransform_.translate;
	halberdTransform_.SetParent(&transform_);
	modelTransform_.rotate = { 0.0f,0.0f,0.0f };
	modelTransform_.scale = { 1.0f,1.0f,1.0f };
	transform_.scale = { 1.0f,1.0f,1.0f };

	halberdLeft_->SetPosition(kBasicHalberdLeftPos);
	halberdLeft_->SetRotate({ 0.0f,0.0f,0.0f });
	halberdLeft_->SetParent(&transform_);
	halberdRight_->SetPosition(kBasicHalberdRightPos);
	halberdRight_->SetRotate({ 0.0f,0.0f,0.0f });
	halberdRight_->SetParent(&transform_);
	Transform transformTemp;
	transformTemp = transform_;
	transformTemp.translate.y += 2.5f;
	currentHP_ -= maxHP_ / 20.0f;
	ParticleManager::GetInstance()->SpawnNumbers(maxHP_ / 20.0f , transformTemp, { 0.5f,0.5f,1.0f });
	if (currentHP_ < 1.0f) {
		currentHP_ = 1.0f;
		DeltaTime::GetInstance()->SetHitStop(0.5f);
		attackRequest_ = Attacks::kLastDown;
		phase_ = Phase::kPhase3;
		AttackInitialize();

	}
}

void Boss::SuperDownUpdate() {
	Transform emitterTransform;
	switch (currentAttackPhase) {
	case 0:
		transform_.translate.y = Easing(kBasicPositionY, kDownStartPosY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.x = Easing(0.0f, kDownStayRotateX / 2.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		destinationHalberdTransform_.translate = Easing(downAnimHalberdPos_, kDownHalberdPos_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(downAnimHalberdRotate_, kDownHalberdRotate_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDonwStartTimer);
		}
		break;
	case 1:
		transform_.translate.y = Easing(kDownStartPosY, kDownStayPosY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		transform_.rotate.x = Easing(kDownStayRotateX / 2.0f, kDownStayRotateX, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kSuperDonwStayTimer);
		}
		break;
	case 2: // 攻撃の前隙.
		emitterTransform = transform_;
		emitterTransform.scale = { kBasicColliderSize.x * 3.0f,0.5f,kBasicColliderSize.z * 3.0f };
		emitterTransform.translate.y += colliderSize_.y;
		downEmitter_->SetTransform(emitterTransform);
		downEmitter_->Update();
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDonwFinsihTimer);
			downAnimHalberdPos_ = destinationHalberdTransform_.translate;
		}
		break;
	case 3:
		transform_.translate.y = Easing(kDownStayPosY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.translate = Easing(kDownHalberdPos_, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kDownHalberdRotate_, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		transform_.rotate.x = Easing(kDownStayRotateX, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			damageAmountRecord_ = 0.0f;
			transform_.rotate.x = 0.0f;
			attackRequest_ = Attacks::kWarp;
			AttackInitialize();
		}
		break;
	}
	downEmitter_->DebugDraw();
}

void Boss::LastDownInitialize() {
	SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
	kMaxAttackTimer = kDonwStartTimer;
	donwAnimHalberdVelocityY_ = 2.0f;
	downAnimHalberdRotate_ = halberdTransform_.rotate;
	downAnimHalberdPos_ = halberdTransform_.translate;
	halberdTransform_.SetParent(&transform_);
	modelTransform_.rotate = { 0.0f,0.0f,0.0f };
	modelTransform_.scale = { 1.0f,1.0f,1.0f };
	transform_.scale = { 1.0f,1.0f,1.0f };
	isColliderActive_ = false;

	halberdLeft_->SetPosition(kBasicHalberdLeftPos);
	halberdLeft_->SetRotate(basicHalberdLeftRotate);
	halberdLeft_->SetParent(&transform_);
	halberdRight_->SetPosition(kBasicHalberdRightPos);
	halberdRight_->SetRotate(basicHalberdRightRotate);
	halberdRight_->SetParent(&transform_);
}

void Boss::LastDownUpdate() {
	Transform emitterTransform;
	switch (currentAttackPhase) {
	case 0:
		transform_.translate.y = Easing(kBasicPositionY, kDownStartPosY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.x = Easing(0.0f, kDownStayRotateX / 2.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		destinationHalberdTransform_.translate = Easing(downAnimHalberdPos_, kDownHalberdPos_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(downAnimHalberdRotate_, kDownHalberdRotate_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDonwStartTimer);
		}
		break;
	case 1:
		transform_.translate.y = Easing(kDownStartPosY, kDownStayPosY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		transform_.rotate.x = Easing(kDownStayRotateX / 2.0f, kDownStayRotateX, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDonwStayTimer);
			isColliderActive_ = true;
		}
		break;
	case 2: // 攻撃の前隙.
		emitterTransform = transform_;
		emitterTransform.scale = { kBasicColliderSize.x * 3.0f,0.5f,kBasicColliderSize.z * 3.0f };
		emitterTransform.translate.y += colliderSize_.y;
		downEmitter_->SetTransform(emitterTransform);
		downEmitter_->Update();

		break;
	}
	downEmitter_->DebugDraw();
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
				bulletShotDirectionTemp_ = GetBulletDire();
			}
		}
		break;
	case 1: // 攻撃の前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 1.0f, kSoundEffect);
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
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 1.0f, kSoundEffect);
			NextAttackPhase(kBounsSpinTimerMax);
			randomRadian = Radian(Random::GetInstance()->RandomFloat(0.0f, 359.0f));
			ProjectileManager::GetInstance()->CreateDiffusionBullet(transform_, Vector3(RadianToVector(randomRadian).x, 0.1f, RadianToVector(randomRadian).y).Normalize() * 10.0f, BulletType::kBounce, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(45.0f), 8);
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
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 1.0f, kSoundEffect);
		}
		break;
	case 1: // ハルバードを高速回転させる.
		destinationHalberdTransform_.rotate = Easing(kDiffusionBulletHalberdStartRotate, kDiffusionBulletHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDiffusionBulletBackTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 1.0f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateDiffusionBullet(halberdTransform_, GetBulletDire() * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(15.0f), 3);

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
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 1.0f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, GetBulletDire() * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 0.5f);
		}
		break;
	case 3: // 攻撃を発射させる間(3回目).
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMovingBulletFinishedGapTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 1.0f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, GetBulletDire() * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 0.5f);
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
	kMaxAttackTimer = kWaveStartGapTimerMax;
	movingBulletTimer_ = 0.0f;
	movingBulletTargetPos = GetMoveAnchorPointFind(kMovingBulletAnchorRadius);
	halberdTransform_.SetParent(&modelTransform_);

}

void Boss::WaveUpdate() {

	switch (currentAttackPhase) {
	case 0: // ハルバードを上昇.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kWaveHalberdStayPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kWaveHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(0.0f, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveStayTimerMax);
		}
		break;
	case 1: // ハルバードを上昇.
		destinationHalberdTransform_.rotate = Easing(kWaveHalberdStartRotate, kWaveHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveAttackTimerMax);
		}
		break;
	case 2: // 攻撃態勢に入りながら急降下.
		destinationHalberdTransform_.translate = Easing(kWaveHalberdStayPos, kWaveHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveAttackGapTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 1.0f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateWave(destinationHalberdTransform_, 25.0f, 1.0f, -1.0f, kCollisionEnemyAttack, 15.0f, 3.0f);
		}
		break;
	case 3: // 攻撃後の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kWaveFinishedGapTimerMax);
		}
		break;
	case 4: // 見た目を戻す.
		destinationHalberdTransform_.translate = Easing(kWaveHalberdAttackPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kWaveHalberdAttackPos, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			modelTransform_.rotate.y = 0.0f;
			halberdTransform_.SetParent(&transform_);
		}
		break;
	}
}

void Boss::SpinningInitialize() {
	spinningBounsTimer_ = 0.0f;
	kMaxAttackTimer = kSpinningStartGapTimerMax;
	attackTempTransform_.translate = { 1.0f,0.0f,0.0f };
	attackTempTransform_.SetParent(&transform_);
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);
	attackTempCollider_->SetColliderType(ColliderType::kBox);
	attackTempCollider_->SetDimensionType(ColliderDimensionType::k3D);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack));
	attackTempCollider_->SetCollisionMask(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer));
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	attackTempCollider_->SetDamage(5.0f);
	attackTempCollider_->SetDamageCoolTime(0.1f);
	attackTempCollider_->SetActive(true);
	spinningRotateY = 0.0f;
	isColliderActive_ = false;
}

void Boss::SpinningUpdate() {
	Transform effectTransform;
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
		spinningBounsTimer_ += deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		if (spinningBounsTimer_ >= kPhase2SpinningBounsTimerMax * difficultyMagnificationTime * dopamineSpeed_) {
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 1.0f, kSoundEffect);
			spinningBounsTimer_ -= kPhase2SpinningBounsTimerMax;
		}

		effectTransform = attackTempTransform_;
		effectTransform.scale = kBasicHalberdColliderSize;
		effectTransform.translate.y = 0.0f;
		SlashEffectCreate(&effectTransform, 3);
		transform_.rotate.y = Easing(preTransform_.rotate.y + kSpinningStartRotateY, preTransform_.rotate.y + kSpinningStartRotateY - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kSpinningSpinTimerMax);
			transform_.rotate.y = preTransform_.rotate.y + kSpinningStartRotateY;
		}
		break;
	case 3: // 回転の最中.
		spinningBounsTimer_ += deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		if (spinningBounsTimer_ >= kPhase2SpinningBounsTimerMax * difficultyMagnificationTime * dopamineSpeed_) {
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 1.0f, kSoundEffect);
			spinningBounsTimer_ -= kPhase2SpinningBounsTimerMax;
		}

		effectTransform = attackTempTransform_;
		effectTransform.scale = kBasicHalberdColliderSize;
		effectTransform.translate.y = 0.0f;
		SlashEffectCreate(&effectTransform, 3);
		transform_.rotate.y = Easing(preTransform_.rotate.y + kSpinningStartRotateY, preTransform_.rotate.y + kSpinningSpinGapRotateY - (Radian(360.0f) * 10.0f * difficultyMagnificationTime * dopamineSpeed_), currentAttackTimer_, kMaxAttackTimer * difficultyMagnificationTime * dopamineSpeed_, EaseType::kConstant);
		if (currentAttackTimer_ >= kMaxAttackTimer * difficultyMagnificationTime * dopamineSpeed_) {
			NextAttackPhase(kSpinningSpinFinnishedTimerMax);
			transform_.rotate.y = preTransform_.rotate.y + kSpinningSpinGapRotateY;
		}
		break;
	case 4: // 回転し終わり.
		effectTransform = attackTempTransform_;
		effectTransform.scale = kBasicHalberdColliderSize;
		effectTransform.translate.y = 0.0f;
		SlashEffectCreate(&effectTransform, 3);
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


	attackTempCollider_->SetTransform(attackTempTransform_);
	attackTempTransform_ = destinationHalberdTransform_;
	if (currentAttackPhase <= 4 && currentAttackPhase >= 2) {
		// 回転時ハルバードに当たり判定を出す.
		transform_.translate += move * deltaTime_ * DifficultyManager::GetInstance()->GetSpeedMagnification() * dopamineSpeed_;
		attackTempCollider_->SetSize({ kBasicHalberdColliderSize.x + 2.0f,kBasicHalberdColliderSize.y,kBasicHalberdColliderSize.z });
		attackTempTransform_.translate.y = -1.8f;
		CollisionManager::GetInstance()->AddColliderList(attackTempCollider_.get());
		attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	}

	attackTempCollider_->DrawCollider();
}

void Boss::PowerSlasherInitialize() {
	kMaxAttackTimer = kPowerSlasherStartGapTimerMax;
	attackTempTransform_.translate = { 1.0f,0.0f,0.0f };
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);
	attackTempCollider_->SetColliderType(ColliderType::kBox);
	attackTempCollider_->SetDimensionType(ColliderDimensionType::k3D);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack));
	attackTempCollider_->SetCollisionMask(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer));
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
			NextAttackPhase(kPowerSlasherStayBlankTimerMax);
		}
		break;
	case 2: // 前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			lenght = targetTransform_->translate - transform_.translate;
			// プレイヤーの位置によって攻撃が変わる.
			if (lenght.Length() < kPowerSlasherNearSlashRadius) {
				// 敵に近い位置なら2に遷移.
				NextAttackPhase(kPowerSlasherDashToSlashTimerMax);
				SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f,1.0f, kSoundEffect);
				SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f,1.0f, kSoundEffect);
			} else {
				// 敵から離れた位置なら3に遷移.
				currentAttackPhase++;
				NextAttackPhase(kPowerSlasherDashTimerMax);
			}
		}
		break;
	case 3: // 攻撃しながら構えなおす.
		destinationHalberdTransform_.translate = Easing(kPowerSlasherHalberdStartPos, kPowerSlasherHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kPowerSlasherHalberdStartRotate, kPowerSlasherHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		powerSlasherHalberdCenter_.rotate.y = Easing(0.0f, Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kPowerSlasherModelStartRotateY, kPowerSlasherModelFinishedRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		SlashEffectCreate(&attackTempTransform_, 3);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			// 7(後隙)に遷移.
			currentAttackPhase++;
			currentAttackPhase++;
			currentAttackPhase++;
			NextAttackPhase(kPowerSlasherStayTimerMax);
		}
		break;
	case 4:  // 突進をする.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		lenght = targetTransform_->translate - transform_.translate;
		// プレイヤーの位置によって攻撃の終わるタイミングが変わる.
		if (lenght.Length() < kPowerSlasherSlashRadius) {
			// 射程圏内に入ったら5に遷移.
			NextAttackPhase(kPowerSlasherDashToSlashTimerMax);
		}

		if (kPowerSlasherSlashRadius + transform_.translate.Length() > movingRadius_) {
			//場外に移動しそうになったら5に遷移.
			NextAttackPhase(kPowerSlasherDashToSlashTimerMax);
		}

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			// 突進時間が終了したら5に遷移.
			NextAttackPhase(kPowerSlasherDashToSlashTimerMax);
		}
		break;
	case 5: // 突進しながら構えなおす.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		destinationHalberdTransform_.translate = Easing(kPowerSlasherHalberdStartPos, kPowerSlasherHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kPowerSlasherHalberdStartRotate, kPowerSlasherHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPowerSlasherDashToSlashTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f, 1.0f,kSoundEffect);
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 1.0f,kSoundEffect);
		}

		if (transform_.translate.Length() > movingRadius_) {
			AttackFinished();
			attackRequest_ = Attacks::kDown;
			AttackInitialize();
		}
		break;
	case 6: // 攻撃を行う.
		powerSlasherHalberdCenter_.rotate.y = Easing(0.0f, Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kPowerSlasherModelStartRotateY, kPowerSlasherModelFinishedRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		SlashEffectCreate(&attackTempTransform_, 3);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPowerSlasherSlashStayTimerMax);
		}
		break;
	case 7: // 後隙(3または6から遷移される).
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPowerSlasherFinishedGapTimerMax);
		}
		break;
	case 8: // 見た目を戻す.
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

	if (currentAttackPhase <= 6 && currentAttackPhase >= 3) {
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
	Transform effectTransform;
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
			//SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
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
		effectTransform.Initialize();
		effectTransform.SetParent(&transform_);
		effectTransform.translate.z = -4.0f;
		effectTransform.translate.y = 2.0f;
		SlashEffectCreate(&effectTransform, 3);
		transform_.translate.y = Easing(kFangAttackAnimPositionY, kFangAttackAttackPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		transform_.rotate.x = Easing(preTransform_.rotate.x, kFangAttackAttackRotateX, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kFangAttackHalberdSpinPos, kFangAttackHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f, 1.0f, kSoundEffect);
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 1.0f, kSoundEffect);
			NextAttackPhase(kFangAttackAttackGapTimerMax);
			FangAttackFangCreate();

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

void Boss::FangAttackFangCreate() {
	float rotateY = transform_.rotate.y - Radian(90.0f);
	float lenght = Vector3(targetTransform_->GetWorldPosition() - transform_.GetWorldPosition()).Length();
	Transform newTransform;
	newTransform.Initialize();
	Vector2 center = { halberdTransform_.GetWorldPosition().x, halberdTransform_.GetWorldPosition().z };

	if (lenght <= kFangAttackRadius) {
		for (uint32_t j = 0; j < kFangAttackLoopCount; j++) {
			uint32_t maxCount = 4 * j;
			float rotateBlank = Random::GetInstance()->RandomFloat(0.1f, (360.0f / maxCount));

			for (uint32_t i = 0; i < maxCount; i++) {
				newTransform.translate.x = Rotate({ kFangAttackRadius * Easing(0.0f,1.0f,static_cast<float>(j),static_cast<float>(kFangAttackLoopCount),EaseType::kConstant),0.0f }, center, (i * (360.0f / maxCount) + rotateBlank)).x;
				newTransform.translate.z = Rotate({ kFangAttackRadius * Easing(0.0f,1.0f,static_cast<float>(j),static_cast<float>(kFangAttackLoopCount),EaseType::kConstant),0.0f }, center, (i * (360.0f / maxCount) + rotateBlank)).y;
				ProjectileManager::GetInstance()->CreateSpike(newTransform, 0, kCollisionEnemyAttack, 15.0f, 3.0f);
			}
		}
	} else {
		ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, Vector3(-RadianToVector(rotateY).x, 0.0f, RadianToVector(rotateY).y) * 20.0f, BulletType::kSpike, kCollisionEnemyAttack, 15.0f, 3.0f);
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
	attackTempCollider_->SetDimensionType(ColliderDimensionType::k3D);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack));
	attackTempCollider_->SetCollisionMask(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer));
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
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 1.0f, kSoundEffect);
		}
		break;
	case 2: // 攻撃.
		SlashEffectCreate(&attackTempTransform_, 3);
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
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 1.0f, kSoundEffect);
		}
		break;
	case 6: // 2段目攻撃.
		SlashEffectCreate(&attackTempTransform_, 3);
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
		transform_.translate.y = Easing(kBasicPositionY, kNearThirdStartPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = Easing(preTransform_.rotate.y, nearAttackPreTransform_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearThirdStayTimerMax);
		}
		break;
	case 9: // 3段目前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearThirdAttackTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f, 1.0f, kSoundEffect);
			SoundManager::GetInstance()->SoundPlay("snd_near_attack_third", 1.0f, 1.0f, kSoundEffect);
		}
		break;
	case 10: // 3段目攻撃.
		SlashEffectCreate(&attackTempTransform_, 3);
		transform_.translate.y = Easing(kNearThirdStartPositionY, kNearThirdAttackPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		modelTransform_.rotate.x = Easing(0.0f, kNearThirdModelAttackRotateX, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		attackTempCollider_->SetDamage(30.0f);
		attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
		CollisionManager::GetInstance()->AddColliderList(attackTempCollider_.get());

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearThirdAttackGapTimerMax);
			ProjectileManager::GetInstance()->CreateWave(transform_, 25.0f, 1.0f, 1.0f, kCollisionEnemyAttack, 20.0f, 3.0f);
			Transform newTransform;
			newTransform.Initialize();
			Vector2 center = { halberdTransform_.GetWorldPosition().x, halberdTransform_.GetWorldPosition().z };

			for (uint32_t j = 0; j < kNearThirdAttackRadiusLoopCount; j++) {
				uint32_t maxCount = 4 * j;
				float rotateBlank = Random::GetInstance()->RandomFloat(0.1f, (360.0f / maxCount));

				for (uint32_t i = 0; i < maxCount; i++) {
					newTransform.translate.x = Rotate({ kNearThirdAttackRadius * Easing(0.0f,1.0f,static_cast<float>(j),static_cast<float>(kNearThirdAttackRadiusLoopCount),EaseType::kConstant),0.0f }, center, (i * (360.0f / maxCount) + rotateBlank)).x;
					newTransform.translate.z = Rotate({ kNearThirdAttackRadius * Easing(0.0f,1.0f,static_cast<float>(j),static_cast<float>(kNearThirdAttackRadiusLoopCount),EaseType::kConstant),0.0f }, center, (i * (360.0f / maxCount) + rotateBlank)).y;
					ProjectileManager::GetInstance()->CreateSpike(newTransform, 0, kCollisionEnemyAttack, 15.0f, 3.0f);
				}
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
		transform_.translate.y = Easing(kNearThirdAttackPositionY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
	attackTempCollider_->DrawCollider();

}

void Boss::StartAnimInitialize() {
	GameCamera::GetInstance()->Reset();
	//GameCamera::GetInstance()->SetPosition({0.0f,3.2f,-70.0f});
	//GameCamera::GetInstance()->SetRotate({0.0f,0.0f,0.0f});
	AnimInitialize();
	halberdTransform_.rotate = kAnimStartHalberdRotate;
	halberdTransform_.translate = kAnimStartHalberdPos;
	transform_.rotate.y = Radian(180.0f);
	hpGauge->SetScale({ 0.0f, 1.0f });
	bossNameScaleX_ = 0.0f;
}

void Boss::PhaseChangeAnimInitialize() {
	GameCamera::GetInstance()->Reset();
	//GameCamera::GetInstance()->SetPosition({0.0f,3.2f,-70.0f});
	//GameCamera::GetInstance()->SetRotate({0.0f,0.0f,0.0f});
	AnimInitialize();

	animCameraCenterTransform_.Initialize();
	animCameraTransform_.Initialize();
	animCameraTransform_.translate.y = kBasicPositionY + 10.0f;
	animCameraTransform_.translate.z = -30.0f;
	animCameraTransform_.SetParent(&animCameraCenterTransform_);
	halberdTransform_.rotate = kAnimPhaseChangeStartHalberdRotate;
	halberdTransform_.translate = kAnimPhaseChangeStartHalberdPos;
	transform_.translate = { 0.0f,kBasicPositionY,0.0f };
	transform_.rotate.y = Radian(0.0f);
	bossNameScaleX_ = 0.0f;
	hpGauge->SetScale({ 0.0f, 1.0f });
	preCameraTransform_ = GameCamera::GetInstance()->GetTransform();
	GameCamera::GetInstance()->SetPosition({});
	animTimerMax_ = kAnimPhaseChangeCameraRotateTimerMax;
	halberdRight_->SetIsActive(false);
	halberdLeft_->SetIsActive(false);
	halberdRight_->SetActive(false);
	halberdLeft_->SetActive(false);
}

void Boss::DeathAnimInitialize() {
	GameCamera::GetInstance()->Reset();
	AnimInitialize();

	animCameraCenterTransform_.Initialize();
	animCameraTransform_.Initialize();
	animCameraTransform_.translate.y = kBasicPositionY + 10.0f;
	animCameraTransform_.translate.z = -30.0f;
	animCameraTransform_.SetParent(&animCameraCenterTransform_);
	halberdTransform_.rotate = kBasicHalberdFarRotate;
	halberdTransform_.translate = kBasicHalberdFarPos;
	transform_.translate = { 0.0f,kBasicPositionY,0.0f };
	transform_.rotate.y = Radian(0.0f);
	hpGauge->SetScale({ 0.0f, 1.0f });
	preCameraTransform_ = GameCamera::GetInstance()->GetTransform();
	GameCamera::GetInstance()->SetPosition({});
	animTimerMax_ = kAnimDeathCameraRotateTimerMax;

	animDeathExplodeTimer_ = 0.0f;
	animDeatExplodeCount_ = 0;
}

void Boss::AnimInitialize() {
	animPhase_ = 0;
	animTimer_ = 0.0f;
	animTimerMax_ = 1.0f;
}

void Boss::NextAnimPhase(float timerMax) {
	animTimer_ = 0;
	animTimerMax_ = timerMax;
	animPhase_++;
}

void Boss::StartAnimationUpdate() {
	GameCamera* camera = GameCamera::GetInstance();

	animTimer_ += deltaTime_;
	switch (animPhase_) {
	case 0:

		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartCameraMoveTimerMax);
			preCameraTransform_ = camera->GetTransform();
			halberdRight_->SetActive(false);
			halberdLeft_->SetActive(false);
		}
		break;
	case 1:
		camera->SetPosition(Easing(preCameraTransform_.translate, kAnimStartCameraMovePos, animTimer_, animTimerMax_, EaseType::kEaseInOut));
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartCameraMoveBlankTimerMax);
		}
		break;
	case 2:
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartHalberdSpawnTimerMax);
		}
		break;
	case 3:
		halberdTransform_.translate = Easing(kAnimStartHalberdPos, kAnimStartHalberdSpawnPos, animTimer_, animTimerMax_, EaseType::kEaseInOut);
		camera->SetPosition(Easing(kAnimStartCameraMovePos, kAnimStartHalberdSpawnCameraPos, animTimer_, animTimerMax_, EaseType::kEaseInOut));
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartHalberdSpawnBlankTimerMax);
		}
		break;
	case 4:
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartHalbardSetPosTimerMax);
		}
		break;
	case 5:
		halberdTransform_.rotate = Easing(kAnimStartHalberdRotate, kAnimStartHalberdSetRotate, animTimer_, animTimerMax_, EaseType::kEaseOut);
		halberdTransform_.translate = Easing(kAnimStartHalberdSpawnPos, kAnimStartHalberdSetPos, animTimer_, animTimerMax_, EaseType::kEaseOut);
		camera->SetPosition(Easing(kAnimStartHalberdSpawnCameraPos, kAnimStartHalberdSetCameraPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartHalbardSetPosBlankTimerMax);
		}
		break;
	case 6:
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartEyeGrownTimerMax);
		}
		break;
	case 7:
		transform_.rotate = Easing({ 0.0f,Radian(180.0f),0.0f }, kAnimStartEyeGrownRotate, animTimer_, animTimerMax_, EaseType::kEaseOut);
		camera->SetPosition(Easing(kAnimStartHalberdSetCameraPos, kAnimStartEyeGrownCameraPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartEyeGrownBlankTimerMax);
		}
		break;
	case 8:
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartJumpTimerMax);
		}
		break;
	case 9:
		transform_.rotate = Easing(kAnimStartEyeGrownRotate, { 0.0f,Radian(360.0f),0.0f }, animTimer_, animTimerMax_, EaseType::kEaseOut);
		transform_.translate.y = Easing(kBasicPositionY, kAnimStartJumpPosY, animTimer_, animTimerMax_, EaseType::kEaseOut);
		halberdTransform_.rotate = Easing(kAnimStartHalberdSetRotate, kAnimStartJumpHalberdRotate, animTimer_, animTimerMax_, EaseType::kEaseOut);
		halberdTransform_.translate = Easing(kAnimStartHalberdSetPos, kAnimStartJumpHalberdPos, animTimer_, animTimerMax_, EaseType::kEaseOut);
		camera->SetPosition(Easing(kAnimStartEyeGrownCameraPos, kAnimStartJumpCameraPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		camera->SetRotate(Easing({ 0.0f,0.0f,0.0f }, kAnimStartJumpCameraRotate, animTimer_, animTimerMax_, EaseType::kEaseOut));

		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartJumpBlankTimerMax);
		}
		break;
	case 10:
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartAttackTimerMax);
			transform_.rotate = { 0.0f,0.0f,0.0f };
		}
		break;
	case 11:
		transform_.translate.y = Easing(kAnimStartJumpPosY, kBasicPositionY, animTimer_, animTimerMax_, EaseType::kEaseOut);
		halberdTransform_.rotate = Easing(kAnimStartJumpHalberdRotate, kAnimStartAttackHalberdRotate, animTimer_, animTimerMax_, EaseType::kEaseOut);
		halberdTransform_.translate = Easing(kAnimStartJumpHalberdPos, kAnimStartAttackHalberdPos, animTimer_, animTimerMax_, EaseType::kEaseOut);
		camera->SetPosition(Easing(kAnimStartJumpCameraPos, kAnimStartAttackCameraPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		camera->SetRotate(Easing(kAnimStartJumpCameraRotate, kAnimStartAttackCameraRotate, animTimer_, animTimerMax_, EaseType::kEaseOut));

		if (animTimer_ >= animTimerMax_) {
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 1.0f, kSoundEffect);
			NextAnimPhase(kAnimStartAttackBlankTimerMax);
			Camera::GetInstance()->CreateShake({ 0.5f,0.5f }, kAnimStartAttackBlankTimerMax);
			InputManager::GetInstance()->SetVibration(1.0f, 1.0f, kAnimStartAttackBlankTimerMax);

			SoundManager::GetInstance()->SoundResume("mus_phase1_intro");
		}
		break;
	case 12:
		bossNameScaleX_ = Easing(0.0f, 1.0f, animTimer_, kAnimPhaseChangeNameShowTimerMax, EaseType::kEaseOut);
		hpGauge->SetScale({ bossNameScaleX_, 1.0f });
		camera->SetPosition(Easing(kAnimStartAttackCameraPos, kAnimStartNameShowCameraPos, animTimer_, animTimerMax_, EaseType::kConstant));

		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartFinishTimerMax);
			transform_.rotate = { 0.0f,0.0f,0.0f };
		}
		break;
	case 13:
		halberdTransform_.rotate = Easing(kAnimStartAttackHalberdRotate, kAnimStartFinishHalberdRotate, animTimer_, animTimerMax_, EaseType::kEaseInOut);
		halberdTransform_.translate = Easing(kAnimStartAttackHalberdPos, kAnimStartFinishHalberdPos, animTimer_, animTimerMax_, EaseType::kEaseInOut);
		camera->SetPosition(Easing(kAnimStartAttackCameraPos, preCameraTransform_.translate, animTimer_, animTimerMax_, EaseType::kEaseInOut));
		camera->SetRotate(Easing(kAnimStartAttackCameraRotate, preCameraTransform_.rotate, animTimer_, animTimerMax_, EaseType::kEaseInOut));

		if (animTimer_ >= animTimerMax_) {
			gGamePhase = GamePhase::kBossPhase1;
		}
		break;
	}
}

void Boss::PhaseChangeAnimationUpdate() {
	GameCamera* camera = GameCamera::GetInstance();

	animTimer_ += deltaTime_;
	switch (animPhase_) {
	case 0:
		animCameraTransform_.translate.y = Easing(kBasicPositionY + 10.0f, kBasicPositionY, animTimer_, animTimerMax_, EaseType::kConstant);
		animCameraCenterTransform_.rotate.y = Easing(0.0f, Radian(360.0f), animTimer_, animTimerMax_, EaseType::kEaseInOut);
		camera->SetRotate({ Easing(Radian(20.0f),0.0f,animTimer_,animTimerMax_,EaseType::kConstant),Easing(0.0f,Radian(360.0f),animTimer_,animTimerMax_,EaseType::kEaseInOut),0.0f });
		camera->SetPosition(animCameraTransform_.GetWorldPosition());


		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimPhaseChangeCameraMoveTimerMax);
		}
		break;
	case 1:
		animCameraTransform_.translate.z = Easing(-30.0f, kAnimPhaseChangeCameraMovePosZ_, animTimer_, animTimerMax_, EaseType::kEaseInOut);

		camera->SetPosition(animCameraTransform_.GetWorldPosition());
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimPhaseChangeHalSpawnTimerMax);
		}
		break;
	case 2:
		halberdTransform_.translate = Easing(kAnimPhaseChangeStartHalberdPos, kAnimPhaseChangeHalSpawnHalberdPos, animTimer_, animTimerMax_, EaseType::kEaseInOut);
		camera->SetPosition(Easing(animCameraTransform_.GetWorldPosition(), kAnimPhaseChangeHalSpawnCameraPos, animTimer_, animTimerMax_, EaseType::kEaseInOut));
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimPhaseChangeHalRotate90degTimerMax);
		}
		break;
	case 3:
		halberdTransform_.rotate.y = Easing(0.0f, Radian(90.0f), animTimer_, animTimerMax_, EaseType::kEaseIn);
		if (animTimer_ >= animTimerMax_) {
			halberdRight_->SetIsActive(true);
			halberdRight_->SetPosition(kAnimPhaseChangeHalSpawnHalberdPos);
			halberdRight_->SetRotate({ 0.0f,Radian(90.0f) ,0.0f });
			NextAnimPhase(kAnimPhaseChangeHalRotate270degTimerMax);
		}
		break;
	case 4:
		halberdTransform_.rotate.y = Easing(Radian(90.0f), Radian(270.0f), animTimer_, animTimerMax_, EaseType::kConstant);
		halberdRight_->SetPosition(Easing(kAnimPhaseChangeHalSpawnHalberdPos, kAnimPhaseChangeHalSpawnHalberdRightPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		if (animTimer_ >= animTimerMax_) {
			halberdLeft_->SetIsActive(true);
			halberdLeft_->SetPosition(kAnimPhaseChangeHalSpawnHalberdPos);
			halberdLeft_->SetRotate({ 0.0f,Radian(270.0f) ,0.0f });
			NextAnimPhase(kAnimPhaseChangeHalRotate360degTimerMax);
		}
		break;
	case 5:
		halberdTransform_.rotate.y = Easing(Radian(270.0f), Radian(360.0f), animTimer_, animTimerMax_, EaseType::kEaseOut);
		halberdLeft_->SetPosition(Easing(kAnimPhaseChangeHalSpawnHalberdPos, kAnimPhaseChangeHalSpawnHalberdLeftPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimPhaseChangeCameraHalRotateTimerMax);
			animCameraTransform_.translate = kAnimPhaseChangeHalSpawnCameraPos;
		}
		break;
	case 6:
		LightManager::GetInstance()->GetDirectionalLightData()->color.SetColorWithoutAlpha(Easing({ 1.0f,1.0f,1.0f }, { 1.0f,0.7f,0.7f }, animTimer_, animTimerMax_, EaseType::kEaseInOut), 1.0f);;
		animCameraCenterTransform_.rotate.y = Easing(0.0f, Radian(360.0f), animTimer_, animTimerMax_, EaseType::kEaseInOut);
		animCameraTransform_.translate.y = Easing(kAnimPhaseChangeHalSpawnCameraPos.y, kBasicPositionY, animTimer_, animTimerMax_, EaseType::kEaseInOut);
		camera->SetPosition(animCameraTransform_.GetWorldPosition());
		camera->SetRotate({ 0.0f,Easing(0.0f,Radian(360.0f),animTimer_,animTimerMax_,EaseType::kEaseInOut),0.0f });
		halberdRight_->SetRotateY(Easing(Radian(90.0f), kAnimPhaseChangeCameraHalRotateHalberdRightRotateY, animTimer_, animTimerMax_, EaseType::kEaseInOut));
		halberdLeft_->SetRotateY(Easing(Radian(-90.0f), kAnimPhaseChangeCameraHalRotateHalberdLeftRotateY, animTimer_, animTimerMax_, EaseType::kEaseInOut));

		halberdTransform_.translate = Easing(kAnimPhaseChangeHalSpawnHalberdPos, kAnimPhaseChangeCameraHalRotateHalberdPos, animTimer_, animTimerMax_, EaseType::kEaseInOut);
		halberdRight_->SetPosition(Easing(kAnimPhaseChangeHalSpawnHalberdRightPos, kAnimPhaseChangeCameraHalRotateHalberdRightPos, animTimer_, animTimerMax_, EaseType::kEaseInOut));
		halberdLeft_->SetPosition(Easing(kAnimPhaseChangeHalSpawnHalberdLeftPos, kAnimPhaseChangeCameraHalRotateHalberdLeftPos, animTimer_, animTimerMax_, EaseType::kEaseInOut));


		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimPhaseChangeCameraHalRotateBlankTimerMax);
		}
		break;
	case 7:
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimPhaseChangeJumpTimerMax);
		}
		break;
	case 8:
		transform_.rotate = Easing({ 0.0f,0.0f,0.0f }, { 0.0f,Radian(360.0f),0.0f }, animTimer_, animTimerMax_, EaseType::kEaseOut);
		transform_.translate.y = Easing(kBasicPositionY, kAnimPhaseChangeJumpPosY, animTimer_, animTimerMax_, EaseType::kEaseOut);
		camera->SetPosition(Easing(animCameraTransform_.GetWorldPosition(), kAnimPhaseChangeJumpCameraPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		camera->SetRotate(Easing({ 0.0f,0.0f,0.0f }, kAnimPhaseChangeJumpCameraRotate, animTimer_, animTimerMax_, EaseType::kEaseOut));

		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimPhaseChangeJumpBlankTimerMax);
		}
		break;
	case 9:
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimPhaseChangeAttackTimerMax);
			transform_.rotate = { 0.0f,0.0f,0.0f };
		}
		break;
	case 10:
		transform_.translate.y = Easing(kAnimPhaseChangeJumpPosY, kBasicPositionY, animTimer_, animTimerMax_, EaseType::kEaseOut);
		halberdTransform_.rotate = Easing({ 0.0f,0.0f,0.0f }, kAnimPhaseChangeAttackHalberdRotate, animTimer_, animTimerMax_, EaseType::kEaseOut);
		halberdTransform_.translate = Easing(kAnimPhaseChangeCameraHalRotateHalberdPos, kAnimPhaseChangeAttackHalberdPos, animTimer_, animTimerMax_, EaseType::kEaseOut);
		camera->SetPosition(Easing(kAnimPhaseChangeJumpCameraPos, kAnimPhaseChangeAttackCameraPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		camera->SetRotate(Easing(kAnimPhaseChangeJumpCameraRotate, kAnimPhaseChangeAttackCameraRotate, animTimer_, animTimerMax_, EaseType::kEaseOut));
		halberdRight_->SetPosition(Easing(kAnimPhaseChangeCameraHalRotateHalberdRightPos, kAnimPhaseChangeAttackHalberdRightPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		halberdRight_->SetRotateX(Easing(0.0f, kAnimPhaseChangeAttackHalberdRotate.x, animTimer_, animTimerMax_, EaseType::kEaseOut));
		halberdLeft_->SetPosition(Easing(kAnimPhaseChangeCameraHalRotateHalberdLeftPos, kAnimPhaseChangeAttackHalberdLeftPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		halberdLeft_->SetRotateX(Easing(0.0f, kAnimPhaseChangeAttackHalberdRotate.x, animTimer_, animTimerMax_, EaseType::kEaseOut));



		if (animTimer_ >= animTimerMax_) {
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 1.0f, kSoundEffect);
			NextAnimPhase(kAnimPhaseChangeAttackBlankTimerMax);
			Camera::GetInstance()->CreateShake({ 0.5f,0.5f }, kAnimPhaseChangeAttackBlankTimerMax);
			InputManager::GetInstance()->SetVibration(1.0f, 1.0f, kAnimPhaseChangeAttackBlankTimerMax);
			currentHP_ = maxHP_ / 2.0f;
		}
		break;
	case 11:
		bossNameScaleX_ = Easing(0.0f, 1.0f, animTimer_, kAnimPhaseChangeNameShowTimerMax, EaseType::kEaseOut);
		hpGauge->SetScale({ bossNameScaleX_, 1.0f });
		camera->SetPosition(Easing(kAnimPhaseChangeAttackCameraPos, kAnimPhaseChangeNameShowCameraPos, animTimer_, animTimerMax_, EaseType::kConstant));

		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimStartFinishTimerMax);
			transform_.rotate = { 0.0f,0.0f,0.0f };
		}
		break;
	case 12:
		halberdTransform_.rotate = Easing(kAnimStartAttackHalberdRotate, kAnimStartFinishHalberdRotate, animTimer_, animTimerMax_, EaseType::kEaseInOut);
		halberdTransform_.translate = Easing(kAnimStartAttackHalberdPos, kAnimStartFinishHalberdPos, animTimer_, animTimerMax_, EaseType::kEaseInOut);
		camera->SetPosition(Easing(kAnimPhaseChangeNameShowCameraPos, preCameraTransform_.translate, animTimer_, animTimerMax_, EaseType::kEaseInOut));
		camera->SetRotate(Easing(kAnimPhaseChangeAttackCameraRotate, preCameraTransform_.rotate, animTimer_, animTimerMax_, EaseType::kEaseInOut));
		halberdRight_->SetPosition(Easing(kAnimPhaseChangeAttackHalberdRightPos, kBasicHalberdRightPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		halberdRight_->SetRotateX(Easing(kAnimPhaseChangeAttackHalberdRotate.x, 0.0f, animTimer_, animTimerMax_, EaseType::kEaseOut));
		halberdRight_->SetRotateY(Easing(kAnimPhaseChangeCameraHalRotateHalberdRightRotateY, 0.0f, animTimer_, animTimerMax_, EaseType::kEaseInOut));
		halberdLeft_->SetPosition(Easing(kAnimPhaseChangeAttackHalberdLeftPos, kBasicHalberdLeftPos, animTimer_, animTimerMax_, EaseType::kEaseOut));
		halberdLeft_->SetRotateX(Easing(kAnimPhaseChangeAttackHalberdRotate.x, 0.0f, animTimer_, animTimerMax_, EaseType::kEaseOut));
		halberdLeft_->SetRotateY(Easing(kAnimPhaseChangeCameraHalRotateHalberdLeftRotateY, 0.0f, animTimer_, animTimerMax_, EaseType::kEaseInOut));

		if (animTimer_ >= animTimerMax_) {
			gGamePhase = GamePhase::kBossPhase2;
			phase_ = Phase::kPhase2;
		}
		break;
	}
}

void Boss::DeathAnimationUpdate() {
	GameCamera* camera = GameCamera::GetInstance();
	Vector3 random = { 0.0f,0.0f,0.0f };
	animTimer_ += deltaTime_;
	switch (animPhase_) {
	case 0:
		if (animDeatExplodeCount_ < kAnimDeathExplodeCountMax) {
			animDeathExplodeTimer_ += deltaTime_;
			if (animDeathExplodeTimer_ >= kAnimDeathExplodeRate) {
				SoundManager::GetInstance()->SoundPlay("snd_explode_mini", 1.0f, 0.25f, kSoundEffect);
				random = Random::GetInstance()->RandomVector3({ -3.0f,-3.0f,-3.0f }, { 3.0f,3.0f,3.0f });
				for (uint32_t i = 0; i < 10; i++) {
					ParticleManager::GetInstance()->SpawnParticles("death_cross", transform_.GetWorldPosition() + random);
				}
				animDeatExplodeCount_++;
				animDeathExplodeTimer_ -= kAnimDeathExplodeRate;
			}
		}

		animCameraTransform_.translate.y = Easing(kBasicPositionY + 10.0f, kBasicPositionY + 5, animTimer_, animTimerMax_, EaseType::kConstant);
		animCameraTransform_.translate.z = Easing(-30.0f, -10.0f, animTimer_, animTimerMax_, EaseType::kConstant);
		animCameraCenterTransform_.rotate.y = Easing(0.0f, Radian(360.0f), animTimer_, animTimerMax_, EaseType::kEaseInOut);
		camera->SetRotate({ Radian(20.0f),Easing(0.0f,Radian(360.0f),animTimer_,animTimerMax_,EaseType::kEaseInOut),0.0f });
		camera->SetPosition(animCameraTransform_.GetWorldPosition());
		transform_.rotate.x = Easing(0.0f, Radian(20.0f), animTimer_, animTimerMax_, EaseType::kEaseInOut);

		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimDeathCameraStayTimerMax);
		}
		break;
	case 1:
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimDeathScaleTimerMax);
		}
		break;
	case 2:
		transform_.scale = Easing({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, animTimer_, animTimerMax_, EaseType::kEaseIn);
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimDeathExplodeTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_explode", 1.0f, 1.0f, kSoundEffect);
			InputManager::GetInstance()->SetVibration(1.0f, 1.0f, kAnimDeathExplodeTimerMax + kAnimDeathExplodeBlankTimerMax);
			for (uint32_t i = 0; i < 30; i++) {
				ParticleManager::GetInstance()->SpawnParticles("death_cross", transform_.GetWorldPosition());
			}
		}
		break;
	case 3:
		animCameraTransform_.translate.y = Easing(kBasicPositionY + 5, kBasicPositionY, animTimer_, animTimerMax_, EaseType::kEaseOut);
		animCameraTransform_.translate.z = Easing(-10.0f, -40.0f, animTimer_, animTimerMax_ + kAnimDeathExplodeBlankTimerMax, EaseType::kEaseOut);
		camera->SetPosition(animCameraTransform_.GetWorldPosition());
		camera->SetRotate({ Easing(Radian(20.0f),0.0f,animTimer_,animTimerMax_,EaseType::kEaseOut),0.0f,0.0f });

		if (animTimer_ >= animTimerMax_) {
			SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f, 1.0f, kSoundEffect);
			SoundManager::GetInstance()->SoundPlay("snd_shine", 1.0f, 1.0f, kSoundEffect);
			NextAnimPhase(kAnimDeathExplodeBlankTimerMax);
			isDeath_ = true;
		}
		break;
	case 4:
		animCameraTransform_.translate.z = Easing(-10.0f, -40.0f, animTimer_ + kAnimDeathExplodeTimerMax, animTimerMax_ + kAnimDeathExplodeTimerMax, EaseType::kEaseOut);
		camera->SetPosition(animCameraTransform_.GetWorldPosition());

		if (animTimer_ >= animTimerMax_) {
			isChangePhase_ = true;
			phase_ = Phase::kFinished;
			NextAnimPhase(kAnimDeathExplodeBlankTimerMax);
		}
		break;
	}
}
