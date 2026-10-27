#include "Boss.h"
#include "MapChipManager.h"
#include "ProjectileManager.h"
#include "GameCamera.h"

void (Boss::* Boss::pInitializeFunc[])() = {
		&Boss::WarpInitialize,
		&Boss::DownInitialize,
		&Boss::ResetBlockInitialize,
		&Boss::BulletInitialize,
		&Boss::DashInitialize,
		&Boss::NearLightningInitialize,
		&Boss::LightningInitialize,
		&Boss::MeteorInitialize,
		&Boss::GroundShakeInitialize,
		&Boss::PhaseChangeAttackInitialize,
};

void (Boss::* Boss::pUpdateFunc[])() = {
		&Boss::WarpUpdate,
		&Boss::DownUpdate,
		&Boss::ResetBlockUpdate,
		&Boss::BulletUpdate,
		&Boss::DashUpdate,
		&Boss::NearLightningUpdate,
		&Boss::LightningUpdate,
		&Boss::MeteorUpdate,
		&Boss::GroundShakeUpdate,
		&Boss::PhaseChangeAttackUpdate,
};

Boss::~Boss() {
	delete targetTransform_;
}

void Boss::Initialize() {
	model_.Initialize("boss");
	model_.SetBlendMode(BlendMode::kNormalCullNone);
	halberdModel_.Initialize("halberd");
	anchorPointCenter_ = Vector3(108.0f, 21.0f, 0.0f);
	anchorPoints_.push_back(anchorPointCenter_);
	anchorPointCenter_ = Vector3(126.0f, 21.0f, 0.0f);
	anchorPoints_.push_back(anchorPointCenter_);

	currentAttack_ = Attacks::kWarp;
	transform_.Initialize();
	transform_.translate.y = kBasicPositionY;
	transform_.translate = Vector3(126.0f, 21.0f, 0.0f);
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

	//switch (DifficultyManager::GetInstance()->GetCurrentDifficulty()) {
	//case kDifficultyEasy:
	//	maxHP_ = 600.0f;
	//	bossName_ = "name_easy";
	//	bossNameMirror_ = "name_easy_mirror";
	//	break;
	//case kDifficultyNormal:
	//	maxHP_ = 600.0f;
	//	bossName_ = "name_normal";
	//	bossNameMirror_ = "name_normal_mirror";
	//	break;
	//case kDifficultyHard:
	//	maxHP_ = 600.0f;
	//	bossName_ = "name_hard";
	//	bossNameMirror_ = "name_hard_mirror";
	//	break;
	//case kDifficultyHell:
	//	maxHP_ = 400.0f;
	//	bossName_ = "name_hard";
	//	bossNameMirror_ = "name_hard_mirror";
	//	break;
	//default:
	//	maxHP_ = 400.0f;
	//	bossName_ = "name_normal";
	//	bossNameMirror_ = "name_normal_mirror";
	//	break;
	//}

	maxHP_ = 600.0f;
	bossName_ = "boss_name";
	bossNameMirror_ = "boss_name";

	currentHP_ = maxHP_;

	hpGauge = std::make_unique<HPGauge>();
	hpGauge->Initialize(&currentHP_, maxHP_, { 800.0f,30.0f });
	hpGauge->SetPosition({ 0.0f,-270.0f });
	hpGauge->SetScale({ 0.0f, 1.0f });
	hpGauge->SetGaugeLine(true);
	hpGauge->SetGaugeLineInterval(10);
	bossNameScaleX_ = 0.0f;

	DifficultyManager::GetInstance()->SetBossHPData(&currentHP_, maxHP_);

	//GameCamera::GetInstance()->SetEnemyTransform(&transform_);

	phase_ = Phase::kPhase1;
	Phase1Initialize();

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

	attackCount_ = 0;

	blockModel_.Initialize("boss_block");
	blockModel_.SetBlendMode(BlendMode::kNormalCullNone);
	modelBlockTransform_.Initialize();
	modelBlockTransform_.SetParent(&modelTransform_);

	isActive_ = true;
	isFight_ = false;
	startTimer_ = 0.0f;

	isVisibleGroundShakeWarningBlock_ = false;
	isVisiblePhaseChangeWarningBlock_ = false;
	usablePhaseChangeAttack_ = true;
}

void Boss::SetAttackData(Attacks attackName, float weight) {
	AttackData newData;
	newData.attackName = attackName;
	newData.continuousCount = 0;
	newData.weight = weight;
	newData.magnification = 1.0f;
	attackDatasPhase1_.push_back(newData);
}

void Boss::Phase1Initialize() {
	attackDatasPhase1_.clear();

	SetAttackData(Attacks::kNearLightning, 1.0f);
	SetAttackData(Attacks::kMeteor, 1.0f);
	SetAttackData(Attacks::kLightning, 1.0f);
	SetAttackData(Attacks::kGroundShake, 1.0f);

	usablePhaseChangeAttack_ = true;

}

void Boss::Phase2Initialize() {
	attackDatasPhase1_.clear();


}

void Boss::Update() {
	if (!isActive_) {
		return;
	}

	if (isDeath_) {
		for (uint32_t i = 0; i < 50; i++) {
			ParticleManager::GetInstance()->SpawnParticles("death_cross", transform_.GetWorldPosition());
		}
		isActive_ = false;
		return;
	}
	if (!isFight_) {
		return;
	}

	if (!isColliderActive_) {
		isColliderActive_ = true;
	}

	if (startTimer_ >= kStartTimerMax_) {
		startTimer_ = kStartTimerMax_;
		bossNameScaleX_ = 1.0f;
	} else {
		startTimer_ += DeltaTime::GetInstance()->GetDeltaTime();
		bossNameScaleX_ = Easing(0.0f, 1.0f, startTimer_, kStartTimerMax_, EaseType::kEaseOut);
	}

	hpGauge->SetScale({ bossNameScaleX_ ,1.0f });

	dopamineSpeed_ = DifficultyManager::GetInstance()->GetDopamineSpeed();

	difficultyMagnificationTime = DifficultyManager::GetInstance()->GetSpeedMagnification();

	deltaTime_ = DeltaTime::GetInstance()->GetGameTime();

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


	if (isColliderActive_) {
		false;
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

}

void Boss::RootUpdate() {
	modelBlockTransform_.rotate.y += kBlockRotate_ * DeltaTime::GetInstance()->GetGameTime();

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

		AttackSelect(attackDatasPhase1_);
	}
}

Vector3 Boss::GetAnchorPointPos(AnchorPointType type) {
	Vector3 anchorPointPos = transform_.translate;
	float leftLenght = 0.0f;
	float rightLenght = 0.0f;
	switch (type) {
	case Boss::AnchorPointType::kLeft:
		anchorPointPos = anchorPoints_[0];
		break;
	case Boss::AnchorPointType::kRight:
		anchorPointPos = anchorPoints_[1];
		break;
	case Boss::AnchorPointType::kNear:
		leftLenght = (anchorPointPos - anchorPoints_[0]).Length();
		rightLenght = (anchorPointPos - anchorPoints_[1]).Length();
		if (leftLenght < rightLenght) {
			anchorPointPos = anchorPoints_[0];
		} else {
			anchorPointPos = anchorPoints_[1];
		}
		break;
	case Boss::AnchorPointType::kFar:
		leftLenght = (anchorPointPos - anchorPoints_[0]).Length();
		rightLenght = (anchorPointPos - anchorPoints_[1]).Length();
		if (leftLenght > rightLenght) {
			anchorPointPos = anchorPoints_[0];
		} else {
			anchorPointPos = anchorPoints_[1];
		}
		break;
	}
	return anchorPointPos;
}

Boss::AnchorPointType Boss::GetAnchorPointType(AnchorPointType type) {
	float leftLenght = 0.0f;
	float rightLenght = 0.0f;
	Vector3 anchorPointPos = transform_.translate;
	AnchorPointType returnType = type;
	switch (type) {
	case Boss::AnchorPointType::kNear:
		leftLenght = (anchorPointPos - anchorPoints_[0]).Length();
		rightLenght = (anchorPointPos - anchorPoints_[1]).Length();
		if (leftLenght < rightLenght) {
			returnType = AnchorPointType::kLeft;
		} else {
			returnType = AnchorPointType::kRight;
		}
		break;
	case Boss::AnchorPointType::kFar:
		leftLenght = (anchorPointPos - anchorPoints_[0]).Length();
		rightLenght = (anchorPointPos - anchorPoints_[1]).Length();
		if (leftLenght > rightLenght) {
			returnType = AnchorPointType::kLeft;
		} else {
			returnType = AnchorPointType::kRight;
		}
		break;
	}
	return returnType;
}

void Boss::AttackSelect(std::vector<AttackData> attackDatas) {
	std::vector<std::pair<Attacks, float>> randomData;
	float weightMax = 0.0f;
	float selectNum;

	if (usablePhaseChangeAttack_) {
		if (GamePhase::GetInstance()->GetCurrentPhase() == GamePhase::kPhase2) {
			attackRequest_ = Attacks::kPhaseChangeAttack;
			superAttackCount_ = 0;
			attackCount_ = 0;
			moveAttackCount_ = 0;
			return;
		}
	}

	if (attackCount_ < kAttackCountMax_) {
		attackCount_++;
		attackRequest_ = Attacks::kBulletShot;
		return;
	}

	for (AttackData& data : attackDatas) {
		float weight = data.weight * std::pow(0.5f, static_cast<float>(data.continuousCount)) * data.magnification * 10000.0f;

		weightMax = weight + weightMax;

		randomData.push_back(std::pair<Attacks, float>(data.attackName, weight));
	}

	selectNum = Random::GetInstance()->RandomFloat(1.0f, weightMax);

	weightMax = 0.0f;

	if (superAttackCount_ >= kSuperAttackCountMax_) {
		superAttackCount_ = 0;
		attackCount_ = 0;
		attackRequest_ = Attacks::kDash;
		moveAttackCount_++;
		return;
	}

	for (std::pair<Attacks, float>& data : randomData) {
		weightMax += data.second;
		if (selectNum < weightMax) {
			attackRequest_ = data.first;
			break;
		}
	}

	attackCount_ = 0;
	superAttackCount_++;
	ClearAttackDatas();
}

void Boss::ClearAttackDatas() {
	for (AttackData& data : attackDatasPhase1_) {
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
}

void Boss::Phase2HalberdFarAttackUpdate() {
	if (useDebugUpdateStop) {
		return;
	}
}

void Boss::Draw() {
	if (!isActive_) {
		return;
	}

	Renderer* renderer = Renderer::GetInstance();
	//model_.Draw(modelTransform_,true);
	renderer->DrawModel(modelTransform_, &model_, true);
	renderer->DrawShadow(modelTransform_, &model_, { 0.0f,0.0f,0.0f,1.0f });
	renderer->DrawModel(modelBlockTransform_, &blockModel_, true);
	renderer->DrawShadow(modelBlockTransform_, &blockModel_, { 0.0f,0.0f,0.0f,1.0f });

	renderer->DrawModel(halberdTransform_, &halberdModel_, true);
	renderer->DrawShadow(halberdTransform_, &halberdModel_, { 0.0f,0.0f,0.0f,1.0f });
#ifdef _DEBUG

	for (Vector3& pos : anchorPoints_) {
		renderer->DrawSphereWireFrame(Transform::GetInitialValue({ 0.1f,0.1f,0.1f }, { 0.0f,0.0f,0.0f }, pos), { 0.5f,0.5f,1.0f,1.0f });
	}

#endif // _DEBUG

	if (isVisibleGroundShakeWarningBlock_) {
		for (uint32_t i = 0; i < 23; i++) {
			uint32_t xIndex = i + 106;
			Transform newTransform;
			newTransform.Initialize();
			for (uint32_t j = 0; j < 32; j++) {
				MapChipType type = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipTypeByIndex(xIndex, j);
				if (type != MapChipType::kBlock) {
					continue;
				}
				uint32_t subId = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipSubIDByIndex(xIndex, j);
				newTransform.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(xIndex, j);
				renderer->DrawBox(newTransform, "skeleton_block_warning", { 1.0f,0.0f,0.0f,1.0f });
				break;
			}
		}
	}

	if (isVisiblePhaseChangeWarningBlock_) {
		for (uint32_t i = 0; i < 23; i++) {
			uint32_t xIndex = i + 106;
			Transform newTransform;
			newTransform.Initialize();
			for (uint32_t j = 0; j < 32; j++) {
				uint32_t subId = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipSubIDByIndex(xIndex, j);
				if (xIndex >= 108 && xIndex <= 127 ) {
					if (j < 16 || j > 18) {
						continue;
					}
				}
				MapChipType type = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipTypeByIndex(xIndex, j);
				if (type != MapChipType::kBlock) {
					continue;
				}

				newTransform.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(xIndex, j);
				renderer->DrawBox(newTransform, "skeleton_block_warning", { 1.0f,0.0f,0.0f,1.0f });
			}
		}
	}

	DrawCollider();

	if (isFight_) {
		hpGauge->Draw();
	}

	renderer->DrawSprite(Transform::GetInitialValue(Vector3(bossNameScaleX_, 1.0f, 1.0f) * 0.5f, { 0.0f,0.0f,0.0f }, { 0.0f,-320.0f,0.0f }), bossName_, { 1.0f,1.0f,1.0f,1.0f });

}

void Boss::OnCollision(Collider* other) {
	float preHP = currentHP_;
	float tempMag = 1.0f;
	std::string sndName = "snd_boss_damage";
	Transform damageTransform = transform_;
	damageTransform.translate += Random::GetInstance()->RandomVector3(-Vector3(kBasicColliderSize.x / 2.0f, 0.5f, kBasicColliderSize.z / 2.0f), Vector3(kBasicColliderSize.x / 2.0f, 0.5f, kBasicColliderSize.z / 2.0f));
	damageTransform.translate.y += 2.5f;
	if (isPlayAttack_) {
		//if (currentAttack_ == Attacks::kAutoHalberd || currentAttack_ == Attacks::kInfinitySlasher) {
		//	tempMag = 0.0f;
		//	sndName = "snd_parry";
		//}
	}

	if (isImmune_) {
		return;
	}

	if (other->GetCollisionAttribute() == CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer)) {
		return;
	}
	//if ((other->GetCollisionAttribute() & CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayerAttack)) == 0x0) {

	if (damageCoolTimer_ <= 0.0f) {
		currentHP_ -= other->GetDamage() * downDamageMangification_ * tempMag;

		DeltaTime::GetInstance()->SetHitStop(0.05f);
		other->SetActive(false);
		damageCoolTimer_ = 0.0f;//other->GetDamageCoolTime();
		SoundManager::GetInstance()->SoundPlay(sndName, 1.0f, 1.0f, kSoundEffect);
		InputManager::GetInstance()->SetVibration(0.1f, 0.1f, 0.25f);

		if (tempMag == 0.0f) {
			ParticleManager::GetInstance()->SpawnNumbers(0.0f, damageTransform, { 0.4f,0.4f,0.4f });
		} else {
			ParticleManager::GetInstance()->SpawnNumbers(other->GetDamage() * downDamageMangification_, damageTransform, { 0.5f,0.5f,1.0f });
		}
	}
	if (GamePhase::GetInstance()->GetCurrentPhase() == GamePhase::kPhase1) {
		if (currentHP_ <= (maxHP_ / 2.0f)) {
			SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
			DeltaTime::GetInstance()->SetHitStop(0.5f);
			GamePhase::GetInstance()->SetCurrentPhase(GamePhase::kPhase2);
		}
	}
	if (!isDeath_) {
		if (currentHP_ <= 0.0f) {
			SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
			DeltaTime::GetInstance()->SetHitStop(0.5f);
			isDeath_ = true;
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
}

void Boss::AttackFinished() {
	isVisibleGroundShakeWarningBlock_ = false;
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

	if (moveAttackCount_ >= kMoveAttackCountMax_) {
		moveAttackCount_ = 0;
		attackRequest_ = Attacks::kResetBlock;
	}
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
			attackRequest_ = Attacks::kResetBlock;
			AttackInitialize();
		}
		break;
	}
}

void Boss::DownInitialize() {
	isColliderActive_ = true;
	SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
	kMaxAttackTimer = kDonwStartTimer;
	donwAnimHalberdVelocityY_ = 2.0f;
	downAnimHalberdRotate_ = halberdTransform_.rotate;
	downAnimHalberdPos_ = halberdTransform_.translate;
	halberdTransform_.SetParent(&transform_);
	modelTransform_.rotate = { 0.0f,0.0f,0.0f };
	modelTransform_.scale = { 1.0f,1.0f,1.0f };
	transform_.scale = { 1.0f,1.0f,1.0f };
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
		transform_.translate.y = Easing(kDownStartPosY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
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

void Boss::ResetBlockInitialize() {
	kMaxAttackTimer = kBlockResetStartGapTimerMax;
}

void Boss::ResetBlockUpdate() {
	switch (currentAttackPhase) {
	case 0: // ワープの始まり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x, basicHalberdRotate.x - Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			currentAttackTimer_ = 0;
			kMaxAttackTimer = kBlockResetFinishedGapTimerMax;
			currentAttackPhase = 1;
			MapChipManager::GetInstance()->ResetBlock(106, 0, 129, 31,usablePhaseChangeAttack_);
			SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
		}

		break;
	case 1: // ワープの終わり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x - Radian(180.0f), basicHalberdRotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			moveAttackCount_ = 0;
			AttackFinished();
		}
		break;
	}
}


void Boss::BulletInitialize() {
	//kMaxAttackTimer = kBulletStartGapTimerMax;
	//bulletShotDirectionTemp_ = { 0.0f,0.0f,1.0f };
	ProjectileManager::GetInstance()->CreateEnemyBlock(transform_, *targetTransform_, CollisionAttributeName::kCollisionEnemyAttack, 10.0f, 1);
	AttackFinished();
}

void Boss::BulletUpdate() {
	return;
}

void Boss::DashInitialize() {
	kMaxAttackTimer = kDashStartGapTimerMax;

}

void Boss::DashUpdate() {
	CollisionMapInfo info;
	switch (currentAttackPhase) {
	case 0: // ハルバードを前に向ける.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kDashHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kBasicHalberdFarRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			dashBeforePosition_ = transform_.translate;
			dashAfterPosition_ = GetAnchorPointPos(AnchorPointType::kFar);
			NextAttackPhase(kDashMoveTimerMax);
			tempp_ = transform_.translate;
		}
		break;
	case 1: // 攻撃の前隙.
		transform_.translate = Easing(dashBeforePosition_, dashAfterPosition_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		tempp_ = transform_.translate;
		info.width = 0.9f;
		info.height = 2.0f;
		info.movementAmount = { ((dashAfterPosition_.x - dashBeforePosition_.x) / kMaxAttackTimer) * DeltaTime::GetInstance()->GetGameTime(),0.0f,0.0f };
		MapChipManager::GetInstance()->MapCollisionNoCenter(transform_.GetWorldPosition(), info);

		if (info.isWallCollision) {
			AttackFinished();
			attackRequest_ = Attacks::kDown;
			AttackInitialize();
		}

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kDashFinishedGapTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
		}
		break;
	case 2: // 見た目を戻す.
		destinationHalberdTransform_.translate = Easing(kDashHalberdPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(kBasicHalberdFarRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::NearLightningInitialize() {
	kMaxAttackTimer = kNearLightningStartGapTimerMax;
}

void Boss::NearLightningUpdate() {
	switch (currentAttackPhase) {
	case 0: // ワープの始まり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x, basicHalberdRotate.x - Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearLightningStayTimerMax);
			float blank = 1.0f;
			if (GetAnchorPointType(AnchorPointType::kNear) == AnchorPointType::kRight) {
				blank = -6.0f;
			}

			for (uint32_t i = 0; i < 5; i++) {
				Transform transform = transform_;
				transform.translate.x += blank + static_cast<float>(i);
				ProjectileManager::GetInstance()->CreateLightning(transform, CollisionAttributeName::kCollisionEnemyAttack, 10.0f, 1.0f);
			}
			SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
		}

		break;
	case 1: // ワープの終わり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x - Radian(180.0f), basicHalberdRotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kNearLightningFinishedGapTimerMax);
		}
		break;
	case 2: // ワープの終わり.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::LightningInitialize() {
	kMaxAttackTimer = kLightningStartGapTimerMax;
}

void Boss::LightningUpdate() {
	switch (currentAttackPhase) {
	case 0: // ワープの始まり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x, basicHalberdRotate.x - Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kLightningStayTimerMax);
			uint32_t blank = static_cast<uint32_t>(Random::GetInstance()->RandomFloat(0.0f, 3.0f));


			for (uint32_t i = 0; i < 23; i++) {
				if (i % 4 == blank || i % 4 == (blank + 1)) {
					Transform transform = transform_;
					transform.translate.x = static_cast<float>(i) + 106.0f;
					ProjectileManager::GetInstance()->CreateLightning(transform, CollisionAttributeName::kCollisionEnemyAttack, 10.0f, 1.0f);
				}
			}
			SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
		}

		break;
	case 1: // ワープの終わり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x - Radian(180.0f), basicHalberdRotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kLightningFinishedGapTimerMax);
		}
		break;
	case 2: // ワープの終わり.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::MeteorInitialize() {
	kMaxAttackTimer = kMeteorStartGapTimerMax;

}

void Boss::MeteorUpdate() {
	switch (currentAttackPhase) {
	case 0: // ワープの始まり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x, basicHalberdRotate.x - Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMeteorStayTimerMax);

			ProjectileManager::GetInstance()->CreateMeteor(*targetTransform_, CollisionAttributeName::kCollisionEnemyAttack, 10.0f, 1.0f);

			SoundManager::GetInstance()->SoundPlay("snd_parry", 1.0f, 1.0f, kSoundEffect);
		}

		break;
	case 1: // ワープの終わり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x - Radian(180.0f), basicHalberdRotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kMeteorFinishedGapTimerMax);
		}
		break;
	case 2: // ワープの終わり.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::GroundShakeInitialize() {
	kMaxAttackTimer = kGroundShakeStartGapTimerMax;
	isVisibleGroundShakeWarningBlock_ = true;
}

void Boss::GroundShakeUpdate() {
	switch (currentAttackPhase) {
	case 0: // ワープの始まり.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kGroundShakeStayTimerMax);
		}

		break;
	case 1: // ワープの終わり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x, basicHalberdRotate.x - Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_drill", 1.0f, 1.0f, kSoundEffect);
			for (uint32_t i = 0; i < 23; i++) {
				uint32_t xIndex = i + 106;
				Transform newTransform;
				newTransform.Initialize();
				for (uint32_t j = 0; j < 32; j++) {
					MapChipType type = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipTypeByIndex(xIndex, j);
					if (type != MapChipType::kBlock) {
						continue;
					}
					uint32_t subId = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipSubIDByIndex(xIndex, j);
					if (subId == 2) {
						continue;
					}
					newTransform.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(xIndex, j);
					MapChipManager::GetInstance()->DeleteBlock(newTransform.translate, 0, 0);
					ProjectileManager::GetInstance()->CreateBullet(newTransform, { 0.0f,5.0f,0.0f }, BulletType::kBlock, CollisionAttributeName::kCollisionEnemyAttack, 10.0f, static_cast<float>(subId));
					break;
				}
			}
			isVisibleGroundShakeWarningBlock_ = false;
			NextAttackPhase(kGroundShakeStayTimerMax);
		}
		break;
	case 2: // ワープの終わり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x - Radian(180.0f), basicHalberdRotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kGroundShakeFinishedGapTimerMax);
		}
		break;
	case 3: // ワープの終わり.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::PhaseChangeAttackInitialize() {
	kMaxAttackTimer = kPhaseChangeStartGapTimerMax;
	isVisiblePhaseChangeWarningBlock_ = true;
}

void Boss::PhaseChangeAttackUpdate() {
	switch (currentAttackPhase) {
	case 0: // ワープの始まり.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhaseChangeStayTimerMax);
		}

		break;
	case 1: // ワープの終わり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x, basicHalberdRotate.x - Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_drill", 1.0f, 1.0f, kSoundEffect);
			for (uint32_t i = 0; i < 23; i++) {
				uint32_t xIndex = i + 106;
				Transform newTransform;
				newTransform.Initialize();
				for (uint32_t j = 0; j < 32; j++) {
					uint32_t subId = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipSubIDByIndex(xIndex, j);
					if (xIndex >= 108 && xIndex <= 127) {
						if (j < 16 || j > 18) {
							continue;
						}
					}
					MapChipType type = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipTypeByIndex(xIndex, j);
					if (type != MapChipType::kBlock) {
						continue;
					}

					newTransform.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(xIndex, j);
					newTransform.translate = MapChipManager::GetInstance()->GetMapChipField()->GetMapChipPositionByIndex(xIndex, j);
					MapChipManager::GetInstance()->SuperDeleteBlock(newTransform.translate);
					ProjectileManager::GetInstance()->CreateBullet(newTransform, { 0.0f,0.0f,0.0f }, BulletType::kBlock, CollisionAttributeName::kCollisionEnemyAttack, 10.0f, static_cast<float>(subId));
				}
			}
			NextAttackPhase(kPhaseChangeStayTimerMax);
			isVisiblePhaseChangeWarningBlock_ = false;
			usablePhaseChangeAttack_ = false;
		}
		break;
	case 2: // ワープの終わり.
		destinationHalberdTransform_.rotate.x = Easing(basicHalberdRotate.x - Radian(180.0f), basicHalberdRotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhaseChangeFinishedGapTimerMax);
		}
		break;
	case 3: // ワープの終わり.
		if (currentAttackTimer_ >= kMaxAttackTimer) {

			AttackFinished();
			attackRequest_ = Attacks::kResetBlock;
			AttackInitialize();
		}
		break;
	}
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
			//gGamePhase = GamePhase::kBossPhase1;
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
			NextAnimPhase(kAnimPhaseChangeHalRotate270degTimerMax);
		}
		break;
	case 4:
		halberdTransform_.rotate.y = Easing(Radian(90.0f), Radian(270.0f), animTimer_, animTimerMax_, EaseType::kConstant);
		if (animTimer_ >= animTimerMax_) {
			NextAnimPhase(kAnimPhaseChangeHalRotate360degTimerMax);
		}
		break;
	case 5:
		halberdTransform_.rotate.y = Easing(Radian(270.0f), Radian(360.0f), animTimer_, animTimerMax_, EaseType::kEaseOut);
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

		halberdTransform_.translate = Easing(kAnimPhaseChangeHalSpawnHalberdPos, kAnimPhaseChangeCameraHalRotateHalberdPos, animTimer_, animTimerMax_, EaseType::kEaseInOut);


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

		if (animTimer_ >= animTimerMax_) {
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
