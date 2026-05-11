#include "ForwardEnemyLeavePhase.h"
#include "ForwardEnemy.h"

void ForwardEnemyLeavePhase::Initialize(){
}

void ForwardEnemyLeavePhase::Update(ForwardEnemy* pEnemy) {
	pEnemy->Translate({ -kSpeed,kSpeed ,0.0f });
}