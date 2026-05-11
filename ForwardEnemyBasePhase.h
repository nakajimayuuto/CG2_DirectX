#pragma once
class ForwardEnemy;

class ForwardEnemyBasePhase {
public:
	virtual void Update(ForwardEnemy* pEnemy) = 0;
};
