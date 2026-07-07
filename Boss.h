#pragma once
#include "Satlib.h"
class Boss : public Collider{
public:
	void Initialize();

	  void Update();

	  void Draw();

	  Transform* GetTransform() { return &transform_; };

	  static void RegisterGlobalVariables();
	  static void ApplyGlobalVariables();

	  Vector3 GetWorldPosition() override { return transform_.GetAffineMatrix().GetMatrixToTranslate(); };

	  void OnCollision([[maybe_unused]] Collider* other)override;
private:
	Model model_;

	std::vector<Vector3> anchorPoints_;

	Vector3 anchorPointCenter_;
};

