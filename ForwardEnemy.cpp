#include "ForwardEnemy.h"
#include "ForwardEnemyApproachPhase.h"
#include "ForwardEnemyLeavePhase.h"
#include "Player.h"
#include "./Scene/GameScene.h"

ForwardEnemy::~ForwardEnemy() {
}

void ForwardEnemy::Initialize(Vector3 position){
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("enemy"));
	transform_.Initialize();
	transform_.translate = position;
	velocity_ = { 0.0f,0.0f,0.0f };
	phase_ = new ForwardEnemyApproachPhase();
	phase_->Initialize(this);
	isAlive_ = true;

	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(kCollisionAttributePlayer);
}

void ForwardEnemy::Update(){
	phase_->Update(this);
}

void ForwardEnemy::Draw(){
	model_.Draw(transform_);
}

void ForwardEnemy::OnCollision(){
	isAlive_ = false;
}

void ForwardEnemy::Fire(EnemyBullet::MoveType type){
	Vector3 velocity(0.0f, 0.0f, -kBulletSpeed);

	//velocity = transform_.GetAffineMatrix().TransformNomal(velocity);

	Vector3 playerPos = player_->GetWorldPosition();
	Vector3 enemyPos = GetWorldPosition();

	velocity = playerPos - enemyPos;

	velocity = velocity.Normalize() * kBulletSpeed;

	BaseBullet* newBullet = new EnemyBullet;
	newBullet->Initialize("enemy_bullet", transform_.translate, velocity);

	dynamic_cast<EnemyBullet*>(newBullet)->SetPlayer(player_);
	dynamic_cast<EnemyBullet*>(newBullet)->SetMoveType(type);

	dynamic_cast<GameScene*>(gameScene_)->AddBullet(newBullet);
}

void ForwardEnemy::Translate(Vector3 translate){
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