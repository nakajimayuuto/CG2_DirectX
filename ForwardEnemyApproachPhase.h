#pragma once
#include "ForwardEnemyBasePhase.h"
#include <cstdint>

class ForwardEnemyApproachPhase : public ForwardEnemyBasePhase {
public:
	void Initialize() override;
	void Update(ForwardEnemy* pEnemy) override;
private:
	static inline float kSpeed = 0.1f;

	static inline uint32_t kFireInterval = 60;

	int32_t fireTimer_ = 0;
};

