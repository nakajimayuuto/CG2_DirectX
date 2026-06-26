#pragma once
#include "BaseCharacter.h"
class Enemy : public BaseCharacter {
public:
	void Initialize() override;

	void Update()override;

	void SetPosition(const Vector3& position) { transform_.translate = position; };

	Vector3 GetWorldPosition() override { return transform_.GetAffineMatrix().GetMatrixToTranslate(); };
private:
	void InitializeRotateGimmick();

	void UpdateRotateGimmick();
private:
	static inline float kRotateYSpeed = Radian(2.0f);
	static inline float kAnimationRotateXSpeed = Radian(2.0f);

	static inline float kSpeed = 0.2f;
};

