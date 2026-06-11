#pragma once
#include "../Satlib.h"
#include "IScene.h"
#include <list>
#include "../Skydome.h"

#include "../Player.h"

class GameScene : public IScene{
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	Player* player_ = nullptr;
};

	//Renderer::Sprite frameSpriteLeft_;
	//Renderer::Sprite frameSpriteRight_;
	//Renderer::Sprite backGroundSprite_;
};