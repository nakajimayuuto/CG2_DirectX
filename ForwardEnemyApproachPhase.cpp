#include "ForwardEnemyApproachPhase.h"
#include "ForwardEnemyLeavePhase.h"
#include "ForwardEnemy.h"

void ForwardEnemyApproachPhase::Initialize() {
	fireTimer_ = kFireInterval;
}

void ForwardEnemyApproachPhase::Update(ForwardEnemy* pEnemy){
	pEnemy->Translate({ 0.0f,0.0f,-kSpeed});
	
	fireTimer_--;
	if (fireTimer_ < 0) {
		pEnemy->Fire();

		fireTimer_ = kFireInterval;
	}

	if (pEnemy->GetPosition().z <= 0.0f) {
		pEnemy->ChangePhase(new ForwardEnemyLeavePhase);
	
	}
}
