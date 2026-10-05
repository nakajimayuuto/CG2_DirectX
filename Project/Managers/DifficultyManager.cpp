#include "DifficultyManager.h"
#include "../Engine/Math/Easing.h"
#include "../Engine/SystemFile/ImGui.h"
DifficultyManager* DifficultyManager::GetInstance() {
	static DifficultyManager instance;
	return &instance;
}

void DifficultyManager::Initialize() {
}

void DifficultyManager::Update() {

#ifdef _DEBUG
	ImGui::Begin("difficultyManager");
	int dif = static_cast<int>(currentDifficulty_);
	ImGui::SliderInt("Difficulty", &dif, 0, kDifficultyCount - 1);
	currentDifficulty_ = static_cast<Difficulty>(dif);
#endif // _DEBUG

	switch (currentDifficulty_) {
	case kDifficultyEasy:
		speedMagnification_ = 0.8f;
		damageMagnification_ = 0.75f;
		break;
	case kDifficultyNormal:
		speedMagnification_ = 1.0f;
		damageMagnification_ = 1.0f;
		break;
	case kDifficultyHard:
		speedMagnification_ = 1.5f;
		damageMagnification_ = 1.5f;
		break;
	case kDifficultyHell:
		speedMagnification_ = 2.0f;
		damageMagnification_ = 2.0f;
		break;
	}

	if (playerHP != nullptr && bossHP != nullptr) {
		dopamineSpeed_ = Easing(kPlayerHPMinDopamineMagnification, 1.0f, *playerHP, playerHPMax, EaseType::kConstant) * Easing(kBossHPMinDopamineMagnification, 1.0f, *bossHP, bossHPMax, EaseType::kConstant);
	} else {
		dopamineSpeed_ = 1.0f;
	}

#ifdef _DEBUG
	ImGui::Text(magic_enum::enum_name(currentDifficulty_).data());
	ImGui::Text("DopamineSpeed:%f\n", dopamineSpeed_);
	ImGui::Text("CurrentBossSpeedMag:%f\nCurrentBossDamageMag:%f\n", speedMagnification_ * dopamineSpeed_, damageMagnification_);
	ImGui::End();
#endif // _DEBUG
}