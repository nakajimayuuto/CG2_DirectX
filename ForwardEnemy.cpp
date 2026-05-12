#include "ForwardEnemy.h"
#include "ForwardEnemyApproachPhase.h"
#include "ForwardEnemyLeavePhase.h"
#include "EnemyBullet.h"
#include "Player.h"

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
	phase_->Initialize(this);

	SetCollisionAttribute(kCollisionAttributeEnemy);
	SetCollisionMask(kCollisionAttributePlayer);
}

void ForwardEnemy::Update(){
	phase_->Update(this);

	BulletRemoveCheck();

	for (BaseBullet* bullet : bullets_) {
		bullet->Update();
	}
}

void ForwardEnemy::BulletRemoveCheck() {
	bullets_.remove_if([](BaseBullet* bullet) {
		if (!bullet->GetIsActive()) {
			delete bullet;
			return true;
		}
		return false;
		});
}

void ForwardEnemy::Draw(){
	model_.Draw(transform_);

	for (BaseBullet* bullet : bullets_) {
		bullet->Draw();
	}
}

void ForwardEnemy::OnCollision(){
}

void ForwardEnemy::Fire(){
	Vector3 velocity(0.0f, 0.0f, -kBulletSpeed);

	//velocity = transform_.GetAffineMatrix().TransformNomal(velocity);

	Vector3 playerPos = player_->GetWorldPosition();
	Vector3 enemyPos = GetWorldPosition();

	velocity = playerPos - enemyPos;

	velocity = velocity.Normalize() * kBulletSpeed;

	BaseBullet* newBullet = new EnemyBullet;
	newBullet->Initialize("white_template", transform_.translate, velocity);

	dynamic_cast<EnemyBullet*>(newBullet)->SetPlayer(player_);

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