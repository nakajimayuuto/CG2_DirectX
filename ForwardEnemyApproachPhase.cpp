#include "ForwardEnemyApproachPhase.h"
#include "ForwardEnemyLeavePhase.h"
#include "ForwardEnemy.h"

ForwardEnemyApproachPhase::~ForwardEnemyApproachPhase(){
	for (TimedCall* timedCall : timedCalls_) {
		delete timedCall;
	}
	timedCalls_.clear();
}

void ForwardEnemyApproachPhase::Initialize(ForwardEnemy* pEnemy) {
	fireTimer_ = kFireInterval;
	FireReset(pEnemy);
}

void ForwardEnemyApproachPhase::Update(ForwardEnemy* pEnemy){
	pEnemy->Translate({ 0.0f,0.0f,-kSpeed});
	
	//fireTimer_--;
	//if (fireTimer_ < 0) {
	//	FireReset(pEnemy);
	//	fireTimer_ = kFireInterval;
	//}

	TimedCallRemoveCheck();

	for (TimedCall* timedCall : timedCalls_) {
		timedCall->Update();
	}

	if (pEnemy->GetPosition().z <= 0.0f) {
		pEnemy->ChangePhase(new ForwardEnemyLeavePhase);
	
	}
}

void ForwardEnemyApproachPhase::FireReset(ForwardEnemy* pEnemy){
	pEnemy->Fire();

	std::function<void(void)> callBack = std::bind(&ForwardEnemyApproachPhase::FireReset, this, pEnemy);

	TimedCall* timedCall = new TimedCall(callBack,kFireInterval);

	timedCalls_.push_back(timedCall);
}

void ForwardEnemyApproachPhase::TimedCallRemoveCheck(){
	timedCalls_.remove_if([](TimedCall* timedCall) {
		if (timedCall->GetIsFinished()) {
			delete timedCall;
			return true;
		}
		return false;
		});
}
