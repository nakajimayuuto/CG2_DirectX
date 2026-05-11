#include "ForwardEnemy.h"

void (ForwardEnemy::* ForwardEnemy::pFunc[])() = {
	&ForwardEnemy::ApproachPhaseUpdate,
	&ForwardEnemy::LeavePhaseUpdate
};

void ForwardEnemy::Initialize(Vector3 position){
	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo("enemy_texture"));
	transform_.Initialize();
	transform_.translate = position;
	velocity_ = { 0.0f,0.0f,-kApproachSpeed };
}

void ForwardEnemy::Update(){
	(this->*pFunc[static_cast<size_t>(phase_)])();

	//switch (phase_){
	//case ForwardEnemy::Phase::kApproach:
	//	ApproachPhaseUpdate();
	//	break;
	//case ForwardEnemy::Phase::kLeave:
	//	LeavePhaseUpdate();
	//	break;
	//}
}

void ForwardEnemy::Draw(){
	model_.Draw(transform_);
}

void ForwardEnemy::ApproachPhaseUpdate(){
	velocity_ = { 0.0f,0.0f,-kApproachSpeed};
	transform_.translate += velocity_;

	if (transform_.translate.z <= 0.0f) {
		phase_ = Phase::kLeave;

	}
}

void ForwardEnemy::LeavePhaseUpdate(){
	velocity_ = {-kLeaveSpeed,kLeaveSpeed ,0.0f};
	transform_.translate += velocity_;
}

void ForwardEnemy::RegisterGlobalVariables() {
	const std::string name = "ForwardEnemy";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	globalVariables->AddValue(name, "ApproachSpeed", kApproachSpeed);
	globalVariables->AddValue(name, "LeaveSpeed", kLeaveSpeed);
}

void ForwardEnemy::ApplyGlobalVariables() {
	const std::string name = "ForwardEnemy";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	kApproachSpeed = globalVariables->GetFloatValue(name, "ApproachSpeed");
	kLeaveSpeed = globalVariables->GetFloatValue(name, "LeaveSpeed");

}