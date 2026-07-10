#include "Boss.h"

Boss::~Boss() {
	delete targetTransform_;
}

void Boss::Initialize() {
	model_.Initialize("creeking");
	anchorPointCenter_ = Vector3(0.0f, 0.5f, 0.0f);
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

	attackRequest_ = std::nullopt;
}

void Boss::Update() {
	deltaTime_ = DeltaTime::GetInstance()->GetDeltaTime();

	ImGui::Begin("BossDebug");
	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("scale", reinterpret_cast<float*>(&transform_.scale), 0.05f, 0.0f, 5.0f);
	ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&imRotate), 1.0f, -360.0f, 360.0f);
	ImGui::DragFloat3("translate", reinterpret_cast<float*>(&transform_.translate), 0.25f, -100.0f, 100.0f);



	transform_.rotate = Radian(imRotate);
	ImGui::End();

	AttackInitialize();

	AttackUpdate();
	
	if (!isPlayAttack_) {
		transform_.rotate.y = atan2( transform_.translate.x - targetTransform_->translate.x,transform_.translate.z - targetTransform_->translate.z);
	}

}

void Boss::Draw() {
	Renderer* renderer = Renderer::GetInstance();
	renderer->DrawModel(transform_, &model_, true);
	renderer->DrawShadow(transform_, &model_, { 0.0f,0.0f,0.0f,1.0f });

	for (Vector3& pos : anchorPoints_) {
		renderer->DrawSphereWireFrame(Transform::GetInitialValue({ 0.1f,0.1f,0.1f }, { 0.0f,0.0f,0.0f }, pos), { 0.5f,0.5f,1.0f,1.0f });
	}
}

void Boss::OnCollision(Collider* other) {
}

void Boss::AttackInitialize() {
	if (!isPlayAttack_ && !attackRequest_) {
		return;
	}

	isPlayAttack_ = true;
	attackRequest_ = std::nullopt;
	currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	kMaxAttackTimer = 0.0f; // 攻撃のタイマー最大値.
	currentAttackPhase = 0; // 攻撃のフェーズ.

	currentAttack_ = attackRequest_.value();
	switch (currentAttack_) {
	case Attacks::kWarp:
		WarpInitialize();
		break;
	case Attacks::kBulletShot:
		break;
	case Attacks::kThreeWayShot:
		break;
	case Attacks::kFireBulletShot:
		break;
	}
}

void Boss::AttackUpdate() {
	if (!isPlayAttack_) {
		return;
	}

	switch (currentAttack_) {
	case Attacks::kWarp:
		WarpUpdate();
		break;
	case Attacks::kBulletShot:
		break;
	case Attacks::kThreeWayShot:
		break;
	case Attacks::kFireBulletShot:
		break;
	}
}

void Boss::AttackFinished() {
	isPlayAttack_ = false;
	attackRequest_ = std::nullopt;
	currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	kMaxAttackTimer = 0.0f; // 攻撃のタイマー最大値.
	currentAttackPhase = 0; // 攻撃のフェーズ.
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
