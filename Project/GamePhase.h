#pragma once


class GamePhase {
public:
	enum GamePhaseType {
		kTutorial,
		kBeforeEnter,
		kPhase1,
		kPhase2,
	};

	static GamePhase* GetInstance() { static GamePhase instance; return &instance; };

	void SetCurrentPhase(GamePhaseType type) { currentPhase_ = type; };

	GamePhaseType GetCurrentPhase() { 
		return currentPhase_; 
	};
private:
	GamePhaseType currentPhase_;

};

