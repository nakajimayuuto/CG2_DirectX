#pragma once
#include "Renderer.h"
#include "ModelManager.h"
#include "IScene.h"

class GameScene : public IScene{
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	bool isModelAutoMove = false;

	bool isSphereAutoMove = false;

	bool useMonsterBall = true;

	int textureNumber_ = 0;

	int soundNumber_ = 0;

	Renderer::Model testModel = Renderer::Model();

	Renderer::Sphere testSphere = Renderer::Sphere();

	Renderer::Sprite testSprite = Renderer::Sprite();

	Transform transformModel{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform transformSphere{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	Transform uvTransformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
};

