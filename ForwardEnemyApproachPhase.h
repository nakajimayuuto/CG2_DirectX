#pragma once
#include "ForwardEnemyBasePhase.h"

class ForwardEnemyApproachPhase : public ForwardEnemyBasePhase {
public:
	void Update(ForwardEnemy* pEnemy) override;
private:
	static inline float kSpeed = 0.1f;
};

