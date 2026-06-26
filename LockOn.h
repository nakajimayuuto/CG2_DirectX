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

	Enemy* GetTarget() { return target_; }
private:
	void TargetLockOn(std::list<std::unique_ptr<Enemy>>& enemies);

	bool OutRange();
private:
	static inline float kDistanceLockOn = 100.0f;

	Enemy* target_ = nullptr;

	Sprite sprite_;
	Transform transform_;


	float minDistance_ = 10.0f;

	float maxDistance_ = 50.0f;

	float angleRange_ = Radian(35.0f);

	bool isLockOn_;
};