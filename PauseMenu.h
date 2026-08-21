#pragma once
#include "Satlib.h"
#include "Fade.h"
class PauseMenu{
public:
	void Initialize();

	void Update();

	void Draw();

	void ShowMenu();

	bool GetIsActive() { return isActive_; };
private:
	bool TriggerUp();
	bool TriggerDown(); 
	bool TriggerSubmit();
	void CheckCurrentPhase();
private:
	bool isActive_;
	float preGameTimeSpeed_;

	float startTimer_;
	static inline float kStartTimerMax = 0.1f;

	bool isFinish_;
	float finishTimer_;
	static inline float kFinishTimerMax = 0.2f;

	int32_t currentSelect_ = 0;
	int32_t maxSelect_ = 0;
	
	std::unique_ptr<Fade> fade_;
};

