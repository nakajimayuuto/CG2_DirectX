#include "ForwardEnemy.h"

void ForwardEnemy::Initialize(Vector3 position){
	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo("enemy_texture"));
	transform_.Initialize();
	transform_.translate = position;
	velocity_ = { 0.0f,0.0f,-kSpeed };
}

void ForwardEnemy::Update(){

	transform_.translate += velocity_;
}

void ForwardEnemy::Draw(){
	model_.Draw(transform_);
}

void ForwardEnemy::RegisterGlobalVariables() {
	const std::string name = "ForwardEnemy";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	globalVariables->AddValue(name, "Speed", kSpeed);
}

void ForwardEnemy::ApplyGlobalVariables() {
	const std::string name = "ForwardEnemy";
	GlobalVariables* globalVariables = GlobalVariables::GetInstance();

	kSpeed = globalVariables->GetFloatValue(name, "Speed");

}