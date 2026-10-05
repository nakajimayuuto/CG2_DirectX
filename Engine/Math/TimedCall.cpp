#include "TimedCall.h"
TimedCall::TimedCall(std::function<void(void)> function, uint32_t time) :
	callFunction_(function),
	time_(time),
	isFinished_(false) {
};

void TimedCall::Update() {
	if (isFinished_) {
		return;
	}

	time_--;
	
	if (time_ <= 0) {
		isFinished_ = true;

		callFunction_();
	}
}	