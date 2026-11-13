#pragma once
#include <list>
#include "IScene.h"
#include "Satlib.h"
#include "Ground.h"
#include "Fade.h"
#include "MapChipManager.h"

class GameScene : public IScene {
public:
	~GameScene();
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:
	std::unique_ptr<Ground> ground_;
	std::unique_ptr<Fade> fade_;
};