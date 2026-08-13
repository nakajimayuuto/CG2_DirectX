#include "GamePhase.h"

TutorialManager* TutorialManager::GetInstance() {
	static TutorialManager instance;
	return &instance;
}

void TutorialManager::Initialize(){
	for (uint32_t i = 0; i < static_cast<uint32_t>(TutorialFlagName::kTestCount); i++) {
		flags_[i] = false;
	}
}
