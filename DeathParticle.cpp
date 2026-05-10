#include "DeathParticle.h"

void DeathParticle::Initialize(){
	for (uint32_t i = 0; i < kNumParticles; i++) {
		models_[i].Initialize(ModelManager::GetInstance()->GetModelInfo("death_particle"));
		transforms_[i].Initialize();
	}

	isFinished_ = true;

	counter_ = 0.0f;

	color_ = { 1.0f,1.0f,1.0f,1.0f };
}

void DeathParticle::Update(){
	if (isFinished_) {
		return;
	}

	for (uint32_t i = 0; i < kNumParticles; i++) {
		Vector3 velocity = { kSpeed,0.0f,0.0f };
		float angle = kAngleUnit * i;
		
		Matrix4x4 matrixRotation = Matrix4x4::MakeRotateZMatrix(angle);

		velocity = matrixRotation.MatrixTransform(velocity);

		transforms_[i].translate += velocity;
	}

	counter_ += 1.0f / 60.0f;

	if (counter_ >= kDuration) {
		counter_ = kDuration;
		isFinished_ = true;
	}

	color_.w = std::clamp(1.0f - (counter_ * (1.0f/kDuration)),0.0f,1.0f);

	for (uint32_t i = 0; i < kNumParticles; i++) {
		models_[i].SetColor(color_);
	}
}

void DeathParticle::Draw(){
	if (isFinished_) {
		return;
	}

	for (uint32_t i = 0; i < kNumParticles; i++) {
		models_[i].Draw(transforms_[i]);
	}
}

void DeathParticle::Start(const Vector3& position){
	for (uint32_t i = 0; i < kNumParticles; i++) {
		transforms_[i].Initialize();
		transforms_[i].translate = position;
	}

	isFinished_ = false;

	counter_ = 0.0f;

	color_ = { 1.0f,1.0f,1.0f,1.0f };
}

void DeathParticle::RegisterGlobalVariables() {

	const char* groupName = "DeathParticle";

	GlobalVariables::GetInstance()->CreateGroup(groupName);

	GlobalVariables::GetInstance()->AddValue(groupName, "Duration",kDuration);

	GlobalVariables::GetInstance()->AddValue(groupName, "Speed", kSpeed);
}

void DeathParticle::ApplyGlobalVariables() {
	const char* groupName = "DeathParticle";

	kDuration = GlobalVariables::GetInstance()->GetFloatValue(groupName, "Duration");

	kSpeed = GlobalVariables::GetInstance()->GetFloatValue(groupName, "Speed");
}