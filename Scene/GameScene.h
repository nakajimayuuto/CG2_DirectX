#pragma once
#include <list>
#include "IScene.h"
#include "../Satlib.h"
#include "../Skydome.h"
#include "../Ground.h"
#include "../Player.h"
#include "../FollowCamera.h"
#include "../Boss.h"

class GameScene : public IScene {
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	void CheckAllCollisions();
private:
	std::unique_ptr<Player> player_;
	std::unique_ptr<Skydome> skydome_;
	std::unique_ptr<Ground> ground_;
	std::unique_ptr<Boss> boss_;

	int32_t testNum_;
};