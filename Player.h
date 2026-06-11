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

	Transform* GetTransform() { return &transform_; };
private:
	bool isMoving_;

	static inline float kSpeed = 0.3f;

	static inline float kCompletionRate = 0.25f;

	float targetRotateY;

	Transform transform_;
	
	Model model_;
};

