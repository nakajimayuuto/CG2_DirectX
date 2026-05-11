#include "ForwardEnemy.h"
#include "ForwardEnemyApproachPhase.h"
#include "ForwardEnemyLeavePhase.h"
#include "EnemyBullet.h"

ForwardEnemy::~ForwardEnemy() {

	for (BaseBullet* bullet : bullets_) {
		delete bullet;
	}
	bullets_.clear();
}

void ForwardEnemy::Initialize(Vector3 position){
	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo("enemy_texture"));
	transform_.Initialize();
	transform_.translate = position;
	velocity_ = { 0.0f,0.0f,0.0f };
	phase_ = new ForwardEnemyApproachPhase();

	Fire();
}

void ForwardEnemy::Update(){
	phase_->Update(this);
}

void ForwardEnemy::Draw(){
	model_.Draw(transform_);
}

void ForwardEnemy::Fire(){
	Vector3 velocity(0.0f, 0.0f, kBulletSpeed);

	velocity = transform_.GetAffineMatrix().TransformNomal(velocity);

	BaseBullet* newBullet = new EnemyBullet;
	newBullet->Initialize("bullet_enemy_texture", transform_.translate, velocity);

	bullets_.push_back(newBullet);
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