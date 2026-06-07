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
	Model model_;
	Transform transform_;

	Sprite sprite_;
	Transform transformSprite_;

	Particles particle;

	Emitter emitter_;

	Transform emitterTransform_;

	uint32_t emitterCount_;

	float emitterFrequency_;

	Skydome* skydome_ = nullptr;

	RECT windowRect;

	bool isParticleUpdate_;

	//Renderer::Sprite frameSpriteLeft_;
	//Renderer::Sprite frameSpriteRight_;
	//Renderer::Sprite backGroundSprite_;
};