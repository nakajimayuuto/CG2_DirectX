#pragma once
#include "ForwardEnemyBasePhase.h"
#include "TimedCall.h"
#include <cstdint>
#include <list>

class ForwardEnemyApproachHomingPhase : public ForwardEnemyBasePhase {
public:
	~ForwardEnemyApproachHomingPhase();
	void Initialize(ForwardEnemy* pEnemy) override;
	void Update(ForwardEnemy* pEnemy) override;
	void FireReset(ForwardEnemy* pEnemy);
private:
	void TimedCallRemoveCheck();
private:
	static inline float kSpeed = 0.1f;

	static inline uint32_t kFireInterval = 60;

	int32_t fireTimer_ = 0;

	std::list<TimedCall*> timedCalls_;
};

