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
	static inline float kSpeed = 0.3f;

	Transform transform_;
	
	Model model_;
};

