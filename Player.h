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
	void InitializeFloatingGimmick();

	void UpdateFloatingGimmick();
private:
	static inline float kSpeed = 0.3f;
	static inline float kCompletionRate = 0.25f;

	static inline uint16_t kFloatingAnimationPeriod_ = 120;
	static inline float kFloatingAnimationStep = 2.0f * std::numbers::pi_v<float> / kFloatingAnimationPeriod_;

	static inline float kFloatingAmplitude = 0.3f;

	float floatingParameter = 0.0f;

	bool isMoving_;

	float targetRotateY;

	Transform transform_;
	Transform transformBody_;
	Transform transformHead_;
	Transform transformRArm_;
	Transform transformLArm_;
	
	Model model_;
	Model modelHead_;
	Model modelLArm_;
	Model modelRArm_;
};

