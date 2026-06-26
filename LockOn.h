#pragma once
#include "Satlib.h"
#include "Enemy.h"
#include <list>
class Player;

class LockOn {
public:
	void Initialize();
	void Update(Player* player,std::list<Enemy*>& enemies);
	void Draw();
	bool GetIsLockOn() { return isLockOn_; };

	Enemy* GetTarget() { return target_; }
private:
	static inline float kDistanceLockOn = 100.0f;

	Enemy* target_ = nullptr;

	Sprite sprite_;
	Transform transform_;

	bool isLockOn_;
};