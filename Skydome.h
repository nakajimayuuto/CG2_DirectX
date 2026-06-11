#pragma once
#include "Satlib.h"
class Skydome{
public:
	void Initialize();

	void Draw();
private:
	Transform transform_;

	Model model_;
};

