#include "ShotEnemy.h"

void ShotEnemy::Initialize(){
	transform_.Initialize();

	baseShotTimerMax_ = 1.0f;
	baseShotTimer_ = 0.0f;
}

void ShotEnemy::Update(){
	ShotUpdate();
}

void ShotEnemy::Draw(){
	Renderer::GetInstance()->DrawSphereWireFrame(transform_, {1.0f,0.5f,0.5f,1.0f});
}
