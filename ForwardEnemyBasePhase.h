#pragma once
class ForwardEnemy;

class ForwardEnemyBasePhase {
public:
	virtual void Initialize(ForwardEnemy* pEnemy) = 0;

	virtual void Update(ForwardEnemy* pEnemy) = 0;
};
