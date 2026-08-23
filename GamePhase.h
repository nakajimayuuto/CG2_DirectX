#pragma once
#include <array>
enum GamePhase {
	kTutorial,
	kGameStartAnim,
	kBossPhase1,
	kBossPhaseChangeAnim,
	kBossPhase2,
	kBossLastJaronaAnim,
	kBossLastJarona,
	kGameClearStage,
};

enum GameProgress {
	kPhase1Clear,
	kPhase2Clear,
};

inline GamePhase gGamePhase;

inline GameProgress gGameProgress;


class TutorialManager {
public:
	enum class TutorialFlagName {
		kFirstJump,
		kMoveTest,
		kAttackTest,
		kDashToJump,
		kDashToAttack,
		kDashJumpTest,
		kFinaleTest,
		kFinaleAnim,
		kTestCount,
	};

	static TutorialManager* GetInstance();

	void Initialize();

	void NextTutorial() { currentFlagName_ = static_cast<TutorialFlagName>(static_cast<uint32_t>(currentFlagName_) + 1); };

	void SetCurrentFlag(bool isFlag) { flags_[static_cast<size_t>(currentFlagName_)] = isFlag; };

	TutorialFlagName GetCurrentFlagName() { return currentFlagName_; };
private:
	std::array<bool, static_cast<size_t>(TutorialFlagName::kTestCount)> flags_;

	TutorialFlagName currentFlagName_;
};