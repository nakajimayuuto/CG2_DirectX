#include "ForwardEnemy.h"
#include "ForwardEnemyApproachPhase.h"
#include "ForwardEnemyLeavePhase.h"

void ForwardEnemy::Initialize(Vector3 position){
	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo("enemy_texture"));
	transform_.Initialize();
	transform_.translate = position;
	velocity_ = { 0.0f,0.0f,0.0f };
	phase_ = new ForwardEnemyApproachPhase();
}

void ForwardEnemy::Update(){
	phase_->Update(this);
}

void ForwardEnemy::Draw(){
	model_.Draw(transform_);
}

void ForwardEnemy::Translate(Vector3 translate){
	transform_.translate += translate;
}

void ForwardEnemy::RegisterGlobalVariables() {
	//const std::string name = "ForwardEnemy";
	//GlobalVariables* globalVariables = GlobalVariables::GetInstance();
	//
	//globalVariables->AddValue(name, "ApproachSpeed", kApproachSpeed);
	//globalVariables->AddValue(name, "LeaveSpeed", kLeaveSpeed);
}

void ForwardEnemy::ApplyGlobalVariables() {
	//const std::string name = "ForwardEnemy";
	//GlobalVariables* globalVariables = GlobalVariables::GetInstance();
	//
	//kApproachSpeed = globalVariables->GetFloatValue(name, "ApproachSpeed");
	//kLeaveSpeed = globalVariables->GetFloatValue(name, "LeaveSpeed");

}