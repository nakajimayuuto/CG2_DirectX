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
	static inline float kCharacterSpeed = 0.2f;

	static inline float kMoveLimitX = 20.0f;
	static inline float kMoveLimitY = 11.0f;
	
	Transform transform_;

	Renderer::ModelBox model_;
};

