#include "ForwardEnemyApproachHomingPhase.h"
#include "ForwardEnemyLeavePhase.h"
#include "ForwardEnemy.h"
#include "EnemyBullet.h"

ForwardEnemyApproachHomingPhase::~ForwardEnemyApproachHomingPhase(){
	for (TimedCall* timedCall : timedCalls_) {
		delete timedCall;
	}
	timedCalls_.clear();
}

void ForwardEnemyApproachHomingPhase::Initialize(ForwardEnemy* pEnemy) {
	fireTimer_ = kFireInterval;
	FireReset(pEnemy);
}

void ForwardEnemyApproachHomingPhase::Update(ForwardEnemy* pEnemy){
	pEnemy->Translate({ 0.0f,0.0f,-kSpeed});

	TimedCallRemoveCheck();

	for (TimedCall* timedCall : timedCalls_) {
		timedCall->Update();
	}

	if (pEnemy->GetPosition().z <= (Camera::GetInstance()->GetPosition().z) - 5.0f) {
		pEnemy->ChangePhase(new ForwardEnemyLeavePhase);
	
	}
}

void ForwardEnemyApproachHomingPhase::FireReset(ForwardEnemy* pEnemy){
	pEnemy->Fire(EnemyBullet::MoveType::kHoming);

	std::function<void(void)> callBack = std::bind(&ForwardEnemyApproachHomingPhase::FireReset, this, pEnemy);

	TimedCall* timedCall = new TimedCall(callBack,kFireInterval);

	timedCalls_.push_back(timedCall);
}

void ForwardEnemyApproachHomingPhase::TimedCallRemoveCheck(){
	timedCalls_.remove_if([](TimedCall* timedCall) {
		if (timedCall->GetIsFinished()) {
			delete timedCall;
			return true;
		}
		return false;
		});
}
