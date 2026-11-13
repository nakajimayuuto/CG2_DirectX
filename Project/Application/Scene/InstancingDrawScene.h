#pragma once
#include "Satlib.h"
#include "IScene.h"
#include "../Ground.h"
#include "../Fade.h"

class InstancingDrawScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	std::unique_ptr<Fade> fade_;

	TestParticle instancing_;
};