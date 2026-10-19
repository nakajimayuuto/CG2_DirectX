#pragma once
#include "Satlib.h"
class Skydome{
public:
	void Initialize();

	void Update();

	void Draw();
private:
	Transform transform_;

	Model model_;

	Vector4 color_;
};

