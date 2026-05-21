#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
#include <list>
class Player;

class LockOn {
public:
	struct TargetLockOn {
		std::weak_ptr<BaseEnemy> target;
		Renderer::Sprite sprite;
		Transform transform;
	};

	void Initialize();
	void Update(std::weak_ptr<Player> player,std::list<std::weak_ptr<BaseEnemy>>& enemies);
	void Draw();
	bool GetIsLockOn() { return isLockOn_; };

	std::weak_ptr<BaseEnemy> GetTarget() { return target_; }
	std::list<std::weak_ptr<Collider>> GetTargets();
private:

	static inline float kDistanceLockOn = 100.0f;

	std::weak_ptr<BaseEnemy> target_;

	Renderer::Sprite sprite_;
	Transform transform_;

	std::list<TargetLockOn> targets_;

	bool isLockOn_;
};