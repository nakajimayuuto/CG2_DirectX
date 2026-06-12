#pragma once
#include "Satlib.h"
#include "BaseCharacter.h"
/// <summary>
/// 自キャラ
/// </summary>
class Player : public BaseCharacter{
public:
	void Initialize();

	void Update();

	void Draw();

	Transform* GetTransform() { return &transform_; };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();
private:
	void InitializeFloatingGimmick();

	void UpdateFloatingGimmick();
private:
	static inline float kSpeed = 0.3f;
	static inline float kCompletionRate = 0.25f;

	static inline uint16_t kFloatingAnimationPeriod = 120;

	static inline float kFloatingAmplitude = 0.3f;

	float floatingParameter = 0.0f;

	bool isMoving_;

	float targetRotateY;

	Transform transformBody_;
	static inline Transform transformHead_;
	static inline Transform transformRArm_;
	static inline Transform transformLArm_;
};

