#pragma once
#include "../Satlib.h"
#include "IScene.h"

class TitleScene : public IScene {
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;
private:

};