#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
#include <list>
class Player;

class LockOn {
public:
	void Initialize();
	void Update(std::weak_ptr<Player> player,std::list<std::weak_ptr<BaseEnemy>>& enemies);
	void Draw();
	bool GetIsLockOn() { return isLockOn_; };

	std::weak_ptr<BaseEnemy> GetTarget() { return target_; }
private:
	static inline float kDistanceLockOn = 100.0f;

	std::weak_ptr<BaseEnemy> target_;

	Renderer::Sprite sprite_;
	Transform transform_;

	bool isLockOn_;
};