#pragma once
#include "../Satlib.h"
#include "IScene.h"

class GameScene : public IScene{
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
};

