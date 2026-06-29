#pragma once
#include "Collision.h"
#include "Shape.h"
#include "CollisionConfig.h"
#include "Transform.h"
#include "../../ContactRecord.h"
class Collider {
public:
	Collider() = default;
	virtual ~Collider() = default;
	virtual void OnCollision([[maybe_unused]] Collider* other);

	void SetRadius(float radius) { radius_ = radius; };
	float GetRadius() const { return radius_; };

	void SetTransform(Transform transform) { transform_ = transform; };
	Transform GetTransform() { return transform_; };
	void SetParent(Transform* transform) { transform_.SetParent(transform); };

	void SetCollisionAttribute(uint32_t collisionAttribute) { collisionAttribute_ = collisionAttribute; };
	void SetCollisionMask(uint32_t collisionMask) { collisionMask_ = collisionMask; };

	uint32_t GetCollisionAttribute() { return collisionAttribute_; };
	uint32_t GetCollisionMask() { return collisionMask_; };

	void SetOnCollisionFunc(void (*func)([[maybe_unused]] Collider* other));

	virtual Vector3 GetWorldPosition() { return transform_.GetAffineMatrix().GetMatrixToTranslate(); };
protected:
	void (*pOnCollision_)([[maybe_unused]] Collider* other) = nullptr;

	Transform transform_ = Transform::GetInitialValue();

	float radius_ = 1.0f;

	// 自分の属性(後々ここはstd::vectorにする)
	uint32_t collisionAttribute_ = 0xFFFFFFFF;

	// どの属性と当たるか(後々ここはstd::vectorにする)
	uint32_t collisionMask_ = 0xFFFFFFFF;
};

