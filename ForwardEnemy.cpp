#include "ForwardEnemy.h"
#include "ForwardEnemyApproachPhase.h"
#include "ForwardEnemyLeavePhase.h"
#include "Player.h"
#include "./Scene/GameScene.h"
#include "RailCameraController.h"

ForwardEnemy::~ForwardEnemy() {
	gameScene_.release();
}

void ForwardEnemy::Initialize(Vector3 position) {
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("enemy"));
	transform_.Initialize();
	transform_.translate = position;
	velocity_ = { 0.0f,0.0f,0.0f };
	phase_ = std::make_unique<ForwardEnemyApproachPhase>();
	phase_->Initialize(this);
	isAlive_ = true;

	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(kCollisionAttributePlayer);
}

void ForwardEnemy::Update() {
	phase_->Update(this);
}

void ForwardEnemy::Draw() {
	model_.Draw(transform_);
}

void ForwardEnemy::OnCollision() {
	isAlive_ = false;
}

void ForwardEnemy::Fire(BaseBullet* bullet) {
	//return;
	Vector3 velocity(0.0f, 0.0f, -kBulletSpeed);

	velocity = transform_.GetAffineMatrix().TransformNomal(velocity);

	Vector3 playerPos = player_.lock().get()->GetWorldPosition();
	Vector3 enemyPos = GetWorldPosition();

	if (dynamic_cast<NormalBullet*>(bullet)) {
		velocity = playerPos - enemyPos;
	}
	velocity = velocity.Normalize() * kBulletSpeed;

	BaseBullet* newBullet = bullet;

	if (dynamic_cast<HomingBullet*>(newBullet)) {
		dynamic_cast<HomingBullet*>(newBullet)->SetTarget(player_);
		newBullet->Initialize("homing_bullet", transform_.translate, velocity);
	} else {
		newBullet->Initialize("normal_bullet", transform_.translate, velocity);
	}

	newBullet->SetCollisionAttribute(kCollisionAttributeEnemy);
	newBullet->SetCollisionMask(kCollisionAttributePlayer);
	newBullet->ChangeColor({ 1.0f,0.0f,0.0f,1.0f });
	dynamic_cast<GameScene*>(gameScene_.get())->AddBullet(newBullet);
}

void ForwardEnemy::Translate(Vector3 translate) {
	transform_.translate += translate;
}

void ForwardEnemy::RegisterGlobalVariables() {
	const std::string name = "ForwardEnemy";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	globalVariables->AddValue(name, "BulletSpeed", kBulletSpeed);
}

void ForwardEnemy::ApplyGlobalVariables() {
	const std::string name = "ForwardEnemy";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	kBulletSpeed = globalVariables->GetFloatValue(name, "BulletSpeed");


}