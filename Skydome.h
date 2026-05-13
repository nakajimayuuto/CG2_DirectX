#pragma once
#include "satlib.h"

class Skydome{
public:
	void Initialize();

	void Update();

	void Draw();
private:
	Renderer::Model model_;
};

