#pragma once
#include "Collision.h"
#include "Shape.h"
#include "CollisionConfig.h"
class Collider {
public:
	Collider() = default;
	virtual ~Collider() = default;
	virtual void OnCollision() {};

	void SetRadius(float radius) { radius_ = radius; };
	float GetRadius() const { return radius_; };

	void SetCollisionAttribute(uint32_t collisionAttribute) { collisionAttribute_ = collisionAttribute; };
	void SetCollisionMask(uint32_t collisionMask) { collisionMask_ = ~collisionMask; };

	uint32_t GetCollisionAttribute() { return collisionAttribute_; };
	uint32_t GetCollisionMask() { return collisionMask_; };

	virtual Vector3 GetWorldPosition() = 0;
private:
	float radius_ = 1.0f;

	uint32_t collisionAttribute_ = 0xFFFFFFFF;
	uint32_t collisionMask_ = 0xFFFFFFFF;
};

