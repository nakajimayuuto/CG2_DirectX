#pragma once
#include "Vector3.h"
#include "Collision.h"

struct Field{
public:
	void Initialize(const Vector3 acceleration,const AABB& area);

	void DebugDraw() const;

	void SetAcceleration(const Vector3& acceleration) { acceleration_ = acceleration; };
	void SetArea(const AABB& area) { area_ = area; };
	void SetIsActive(bool isActive) { isActive_ = isActive; };

	Vector3 GetAcceleration() const { return acceleration_; };
	AABB GetArea() const{ return area_; }
	bool GetIsActive() const { return isActive_; };
private:
	Vector3 acceleration_;
	AABB area_;

	bool isActive_;
};

