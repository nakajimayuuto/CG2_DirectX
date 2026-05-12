#include "ForwardEnemyLeavePhase.h"
#include "ForwardEnemy.h"

void ForwardEnemyLeavePhase::Initialize(ForwardEnemy* pEnemy){
	(void)pEnemy;
}

void ForwardEnemyLeavePhase::Update(ForwardEnemy* pEnemy) {
	if (pEnemy->GetPosition().x >= 0.0f) {
		pEnemy->Translate({ kSpeed,kSpeed ,0.0f });
	} else {
		pEnemy->Translate({ -kSpeed,kSpeed ,0.0f });
	}
}