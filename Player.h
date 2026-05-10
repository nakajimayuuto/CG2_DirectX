#pragma once
#include "Satlib.h"
/// <summary>
/// 自キャラ
/// </summary>
class Player{
public:
	void Initialize();

	void Update();

	void Draw();
private:
	Transform transform_;

	Renderer::ModelBox model_;
};

