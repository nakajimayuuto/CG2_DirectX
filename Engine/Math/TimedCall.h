#pragma once
#include <functional>
#include <cstdint>

class TimedCall{
public:
	TimedCall(std::function<void(void)> function, uint32_t time);

	void Update();

	bool GetIsFinished() { return isFinished_; };
private:
	std::function<void(void)> callFunction_;

	uint32_t time_;

	bool isFinished_;
};

