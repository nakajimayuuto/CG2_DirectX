#include "ForwardEnemyLeavePhase.h"
#include "ForwardEnemy.h"

void ForwardEnemyLeavePhase::Initialize(ForwardEnemy* pEnemy){
	(void)pEnemy;
}

void ForwardEnemyLeavePhase::Update(ForwardEnemy* pEnemy) {
	pEnemy->Translate({ -kSpeed,kSpeed ,0.0f });
}