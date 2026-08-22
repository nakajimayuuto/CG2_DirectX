#pragma once
#include "Satlib.h"
class MirrorHalberd : public Collider{
public:
	void Initialize();

	void Update();

	void Draw();

	void SetPosition(const Vector3& position) { transform_.translate = position; };

	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; };

	void SetRotateX(float rotateX) { transform_.rotate.x = rotateX; };
	void SetRotateY(float rotateY) { transform_.rotate.y = rotateY; };
	void SetRotateZ(float rotateZ) { transform_.rotate.z = rotateZ; };

	void SetTransform(const Transform& transform) { transform_ = transform; };

	void SetIsActive(bool isActive) { isActive_ = isActive; };

	bool GetIsActive() { return isActive_; };
	bool GetIsColliderActive() { return isColliderActive_; };

	Vector3 GetPosition() { return transform_.translate; };
	Vector3 GetRotate() { return transform_.rotate; };

	Transform GetTransform() { return transform_; };

	void SetParent(Transform* transform) { transform_.SetParent(transform); };
private:
	Model halberdModel_;

	bool isActive_;
};

