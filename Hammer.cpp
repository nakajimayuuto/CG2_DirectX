#include "Hammer.h"
#include "Enemy.h"
void Hammer::Initialize() {
	workAttack_.comboNext = false;
	workAttack_.attackParameter = 0.0f;
	workAttack_.comboIndex = 0;
	workAttack_.inComboPhase = 0;

	model_.Initialize("hammer_of_justice");
	transform_.Initialize();
	transform_.rotate = { 0.0f,0.0f,0.0f };
	beforeHammerRotate_ = transform_.rotate;
	workAttack_.inComboPhase = 0;
	isFinished_ = false;
	transform_.SetParent(transformTarget_);
	// 01.振りかぶり時間.
	// 02.ため時間.
	// 03.攻撃時間.
	// 04.硬直時間.
	// 05.振りかぶり移動速度.
	// 06.ため移動速度.
	// 07.攻撃移動速度.
	kConstAttacks_[0] = { 0.0f,0.0f,kStampAnimationMaxTime,0.0f,0.0f,0.0f,0.15f };
	kConstAttacks_[1] = { 0.3f,0.2,0.3f,0.0f,0.2f,0.0f,0.0f };
	kConstAttacks_[2] = { 0.3f,0.2f,0.3f,0.5f,0.2f,0.0f,0.0f };

	collisionAttribute_ = kCollisionAttributePlayer;
	collisionMask_ = kCollisionAttributeEnemy;

	record_ = std::make_unique<ContactRecord>();
	record_->Clear();

	colliderHammer_ = std::make_unique<Collider>();
	colliderHammer_->SetTransform(Transform::GetInitialValue({1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,5.0f,0.0f}));
	colliderHammer_->SetParent(&transform_);
	colliderHammer_->SetRadius(radius_);
	colliderHammer_->SetCollisionAttribute(collisionAttribute_);
	colliderHammer_->SetCollisionMask(collisionMask_);
	colliderHammer_->SetOnCollisionFunc(pOnCollision_);
}

void Hammer::Update() {
	// ここに処理を追加
	workAttack_.attackParameter += DeltaTime::GetInstance()->GetDeltaTime();

	if (workAttack_.attackParameter > GetSumComboTime(workAttack_.comboIndex)) {
		if (workAttack_.comboNext) {
			beforeHammerRotate_ = transform_.rotate;
			workAttack_.comboNext = false;
			workAttack_.attackParameter = 0.0f;
			workAttack_.inComboPhase = 0;
			workAttack_.comboIndex++;
			record_->Clear();
		} else {
			transformTarget_->rotate = { 0.0f,0.0f,0.0f };
			transform_.rotate = { 0.0f,0.0f,0.0f };
			isFinished_ = true;
		}
	}

	if (workAttack_.comboIndex < kComboNum) {
		if (InputManager::GetInstance()->TriggerPadButton(PadButtons::INPUT_L1) || InputManager::GetInstance()->TriggerMouse(MouseButtons::MOUSE_LEFT)) {
			workAttack_.comboNext = true;
		}
	} else {
		transformTarget_->rotate = { 0.0f,0.0f,0.0f };
		transform_.rotate = { 0.0f,0.0f,0.0f };
		isFinished_ = true;
	}

	switch (workAttack_.comboIndex) {
	case 0:
		transform_.rotate.x = Easing(kStartHammerRotateX, kStampHammerRotateX, workAttack_.attackParameter, kConstAttacks_[workAttack_.comboIndex].swingTime, EaseType::kEaseInBack);
		break;
	case 1:
		switch (workAttack_.inComboPhase) {
		case 0:
			transform_.rotate = Easing(beforeHammerRotate_, kRollingStartHammerRotate, workAttack_.attackParameter, kConstAttacks_[workAttack_.comboIndex].chargeTime, EaseType::kEaseIn);

			if (workAttack_.attackParameter > kConstAttacks_[workAttack_.comboIndex].chargeTime) {
				beforeHammerRotate_ = transform_.rotate;
				workAttack_.inComboPhase++;
			}
			break;
		case 1:
			transform_.rotate = Easing(beforeHammerRotate_, kRollingSwingHammerRotate, workAttack_.attackParameter - kConstAttacks_[workAttack_.comboIndex].chargeTime, kConstAttacks_[workAttack_.comboIndex].swingTime, EaseType::kEaseIn);
			break;
		}
		break;
	case 2:
		switch (workAttack_.inComboPhase) {
		case 0:
			transform_.rotate = Easing(beforeHammerRotate_, { 0.0f,0.0f,0.0f }, workAttack_.attackParameter, kConstAttacks_[workAttack_.comboIndex].chargeTime, EaseType::kEaseIn);

			if (workAttack_.attackParameter > kConstAttacks_[workAttack_.comboIndex].chargeTime) {
				beforeHammerRotate_ = transformTarget_->rotate;
				workAttack_.inComboPhase++;
			}
			break;
		case 1:
			transformTarget_->rotate = Easing(beforeHammerRotate_, kExtraSwingHammerRotate, workAttack_.attackParameter - kConstAttacks_[workAttack_.comboIndex].chargeTime, kConstAttacks_[workAttack_.comboIndex].swingTime, EaseType::kEaseIn);
			break;
		}
		break;
	}

	//hammerAnimationTimer_ += DeltaTime::GetInstance()->GetDeltaTime();
	//
	//transformHammer_.rotate.x = Easing(kStartHammerRotateX, kStampHammerRotateX, hammerAnimationTimer_, kStampAnimationMaxTime, EaseType::kEaseInBack);
	//
	//if (hammerAnimationTimer_ > kStampAnimationMaxTime) {
	//	behaviorRequest_ = Behavior::kRoot;
	//}
}

float Hammer::GetSumComboTime(uint32_t index) {
	return kConstAttacks_[index].anticipationTime + kConstAttacks_[index].chargeTime + kConstAttacks_[index].recoveryTime + kConstAttacks_[index].swingTime;
}

void Hammer::Draw() {
	Renderer::GetInstance()->DrawModel(transform_,&model_);
}

void Hammer::OnCollision([[maybe_unused]] Collider* other) {
//	emitter_->SetTransform(transform_.GetAffineMatrix().GetMatrixToTransform());
//	emitter_->CreateParticle();
}

void Hammer::pOnCollision_(Collider* other){
	if (Enemy* enemy = static_cast<Enemy*>(other)) {
		uint32_t serialNumber = enemy->GetSerialNumber();
		
		if (record_->RecordCheck(serialNumber)) {
			return;
		}

		record_->AddRecord(serialNumber);
	}
	emitter_->SetTransform(other->GetTransform());
	emitter_->CreateParticle();
}
