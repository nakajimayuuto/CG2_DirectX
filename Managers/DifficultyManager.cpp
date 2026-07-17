#include "DifficultyManager.h"
#include "../Engine/Math/Easing.h"
DifficultyManager* DifficultyManager::GetInstance() {
	static DifficultyManager instance;
	return &instance;
}

void DifficultyManager::Initialize() {
	
}

void DifficultyManager::Update() {
	dopamineSpeed_ = Easing(0.75f,1.0f,*playerHP,playerHPMax,EaseType::kConstant) * Easing(1.5f, 1.0f, *bossHP, bossHPMax, EaseType::kConstant);
}