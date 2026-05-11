#include "ForwardEnemyApproachPhase.h"
#include "ForwardEnemyLeavePhase.h"
#include "ForwardEnemy.h"

void ForwardEnemyApproachPhase::Update(ForwardEnemy* pEnemy){
	pEnemy->Translate({ 0.0f,0.0f,-kSpeed});
	
	if (pEnemy->GetPosition().z <= 0.0f) {
		pEnemy->ChangePhase(new ForwardEnemyLeavePhase);
	
	}
}
