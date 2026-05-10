#pragma once
//#include "../Engine/Renderer/Renderer.h"
//#include "../Managers/ModelManager.h"
#include "../Satlib.h"
#include "IScene.h"

class GameScene : public IScene{
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	Renderer::Model testModel_;

	Transform testTransform_;
};

