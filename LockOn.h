#pragma once
#include "Satlib.h"
#include "Enemy.h"
#include <list>
class Player;

class LockOn {
public:
	void Initialize();
	void Update(std::weak_ptr<Player> player,std::list<std::weak_ptr<Enemy>>& enemies);
	void Draw();
	bool GetIsLockOn() { return isLockOn_; };

	std::weak_ptr<Enemy> GetTarget() { return target_; }
private:
	static inline float kDistanceLockOn = 100.0f;

	std::weak_ptr<Enemy> target_;

	Sprite sprite_;
	Transform transform_;

	bool isLockOn_;
};