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
	bool isModelAutoMove = false;

	bool isMultiModelAutoMove = false;

	bool isSphereAutoMove = false;

	bool useMonsterBall = true;

	int textureNumber_ = 0;

	int soundNumber_ = 0;

	Renderer::Model testModel;

	Renderer::Model testMultiModel = Renderer::Model();

	Renderer::Sphere testSphere = Renderer::Sphere();

	Renderer::Sprite testSprite = Renderer::Sprite();

	Renderer::Box testBox = Renderer::Box();

	Transform transformModel{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform transformMultiModel{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform transformSphere{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
};

