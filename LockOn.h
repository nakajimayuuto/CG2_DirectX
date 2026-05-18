#pragma once
#include "Satlib.h"
#include "BaseEnemy.h"
#include <list>
class Player;

class LockOn {
public:
	void Initialize();
	void Update(Player* player,std::list<BaseEnemy*>& enemies);
	void Draw();
	bool GetIsLockOn() { return isLockOn_; };

	BaseEnemy* GetTarget() { return target_; }
private:
	static inline float kDistanceLockOn = 100.0f;

	BaseEnemy* target_ = nullptr;

	Renderer::Sprite sprite_;
	Transform transform_;

	bool isLockOn_;
};