#pragma once
//#include "../Engine/Renderer/Renderer.h"
//#include "../Managers/ModelManager.h"
#include "../Satlib.h"
#include "IScene.h"
#include <list>
#include "../Skydome.h"

class GameScene : public IScene {
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
private:
	Renderer::Model model_;
	Transform transform_;

	Renderer::Sprite sprite_;
	Transform transformSprite_;

	Skydome* skydome_ = nullptr;

	RECT windowRect;

	Renderer::Sprite frameSpriteLeft_;
	Renderer::Sprite frameSpriteRight_;
	Renderer::Sprite backGroundSprite_;
};