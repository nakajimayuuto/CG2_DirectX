#pragma once
#include "BaseCharacter.h"
class Enemy : public BaseCharacter {
public:
	void Initialize() override;

	void Update()override;
private:
	void InitializeRotateGimmick();

	void UpdateRotateGimmick();
private:
	static inline float kRotateYSpeed = Radian(2.0f);
	static inline float kAnimationRotateXSpeed = Radian(2.0f);

	static inline float kSpeed = 0.2f;
};

