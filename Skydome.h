#pragma once
#include "Satlib.h"
class Skydome{
public:
	void Initialize();

	void Update();

	void Draw();
private:
	Transform transform_;

	Renderer::Model model_;
};

