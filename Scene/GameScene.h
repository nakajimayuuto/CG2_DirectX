#pragma once
#include "../Satlib.h"
#include "IScene.h"
#include <list>
#include "../Skydome.h"

class GameScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
private:
};

