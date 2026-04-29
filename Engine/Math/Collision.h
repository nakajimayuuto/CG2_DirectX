#pragma once
#include "Shape.h"

class Collision{
public:
	static bool SphereToSphere(const Sphere& sphere1,const Sphere& sphere2);

	static bool SphereToPlane(const Sphere sphere, const Plane& plane);

	static bool LineToPlane(const Line& line,const Plane& plane);

	static bool RayToPlane(const Ray& ray, const Plane& plane);

	static bool SegmentToPlane(const Segment& segment, const Plane& plane);

	static bool SegmentToTriangle(const Segment& segment, const Triangle& triangle);

	static bool AABBToAABB(const AABB& aabb1,const AABB& aabb2);

	static bool AABBToSphere(const AABB& aabb, const Sphere sphere);

	static bool AABBToSegment(const AABB& aabb, const Segment& segment);

	static bool AABBToLine(const AABB& aabb, const Line& line);

	static bool AABBToRay(const AABB& aabb, const Ray& ray);

	static bool OBBToSphere(const OBB& obb, const Sphere& sphere);

	static bool OBBToLine(const OBB& obb, const Line& line);

	static bool OBBToRay(const OBB& obb, const Ray& ray);

	static bool OBBToSegment(const OBB& obb, const Segment& segment);

	// ムズすぎ未完成.
	static bool OBBToOBB(const OBB& obb1,const OBB& obb2);
};
