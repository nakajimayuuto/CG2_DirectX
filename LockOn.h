#pragma once
#include "Satlib.h"
#include "Enemy.h"
#include <list>
class Player;

class LockOn {
public:
	void Initialize();
	void Update( std::list<std::unique_ptr<Enemy>>& enemies);
	void Draw();
	bool GetIsLockOn() { return isLockOn_; };

	const Enemy* GetTarget() { return target_; }
private:
	void TargetLockOn(std::list<std::unique_ptr<Enemy>>& enemies);
private:
	static inline float kDistanceLockOn = 100.0f;

	const Enemy* target_ = nullptr;

	Sprite sprite_;
	Transform transform_;


	float minDistance_ = 10.0f;

	float maxDistance_ = 10.0f;

	float angleRange_ = Radian(20.0f);

	bool isLockOn_;
};