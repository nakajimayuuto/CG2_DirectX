#pragma once
#include "ForwardEnemyBasePhase.h"

class ForwardEnemyLeavePhase : public ForwardEnemyBasePhase {
public:
	void Initialize(ForwardEnemy* pEnemy) override;
	void Update(ForwardEnemy* pEnemy) override;
private:
	static inline float kSpeed = 0.1f;
};

