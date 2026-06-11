#pragma once
#include "../Satlib.h"
#include "IScene.h"
#include <list>
#include "../Skydome.h"
#include "../Ground.h"
#include "../FollowCamera.h"
#include "../Player.h"

class GameScene : public IScene{
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	std::unique_ptr<Player> player_;
	std::unique_ptr<Skydome> skydome_;
	std::unique_ptr<Ground> ground_;
	std::unique_ptr<FollowCamera> followCamera_;
};