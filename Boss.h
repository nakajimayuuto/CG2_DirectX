#pragma once
#include "Satlib.h"
class Boss : public Collider{
public:
	~Boss();
	void Initialize();

	  void Update();

	  void Draw();

	  Transform* GetTransform() { return &transform_; };

	  void SetTargetTransform(Transform* transform);

	  static void RegisterGlobalVariables();
	  static void ApplyGlobalVariables();

	  Vector3 GetWorldPosition() override { return transform_.GetAffineMatrix().GetMatrixToTranslate(); };

	  void OnCollision([[maybe_unused]] Collider* other)override;
private:
	Model model_;

	std::vector<Vector3> anchorPoints_;

	Vector3 anchorPointCenter_;

	Transform* targetTransform = nullptr;

	void (*pAttackInitialize)() = nullptr;
	void (*pAttackUpdate)() = nullptr;
private:
	void WarpInitialize();

	void WarpUpdate();
};

