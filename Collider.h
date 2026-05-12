#pragma once
#include "./Engine/Math/Collision.h"
#include "./Engine/Math/Shape.h"
class Collider {
public:
	virtual void OnCollision() {};

	void SetRadius(float radius) { radius_ = radius; };
	float GetRadius() const { return radius_; };

	virtual Vector3 GetWorldPosition() = 0;
private:
	float radius_ = 1.0f;
};

