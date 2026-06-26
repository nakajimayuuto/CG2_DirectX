#pragma once
#include <list>
#include "IScene.h"
#include "../Satlib.h"
#include "../Skydome.h"
#include "../Ground.h"
#include "../FollowCamera.h"
#include "../Player.h"
#include "../Enemy.h"
#include "../LockOn.h"

class GameScene : public IScene{
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	void CheckAllCollisions();
private:
	std::unique_ptr<Player> player_;
	std::list<std::unique_ptr<Enemy>> enemies_;
	std::unique_ptr<Skydome> skydome_;
	std::unique_ptr<Ground> ground_;
	std::unique_ptr<FollowCamera> followCamera_;
	std::unique_ptr<LockOn> lockOn_;
};