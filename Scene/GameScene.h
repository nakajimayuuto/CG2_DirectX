#pragma once
#include "../Satlib.h"
#include "IScene.h"

#include "../Player.h"

class GameScene : public IScene{
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void RegisterGlobalVariables();
	void ApplyGlobalVariables();
private:
	Player* player_ = nullptr;
};

