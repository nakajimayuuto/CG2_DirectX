#pragma once
//#include "../Engine/Renderer/Renderer.h"
//#include "../Managers/ModelManager.h"
#include "../Satlib.h"
#include "IScene.h"
#include <list>
#include "../Skydome.h"
#include "../FakeWindow.h"

class GameScene : public IScene {
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	void CreateFakeWindow();
private:
	Renderer::Model model_;
	Transform transform_;

	Renderer::Sprite sprite_;
	Transform transformSprite_;
	Transform transformSpriteTest2_;

	Skydome* skydome_ = nullptr;

	Renderer::Sprite backGroundSprite_;

	std::vector<std::unique_ptr<FakeWindow>> fakeWindows_;

	bool isRotate_;

	Transform cameraRotateCenter_;
	Transform newCameraTransform_;
};