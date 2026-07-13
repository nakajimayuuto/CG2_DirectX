#define NOMINMAX
#include "Collision.h"
#include <cmath>
#include "Math.h"
#include <algorithm>
#include <vector>

bool Collision::SphereToSphere(const Sphere& sphere1, const Sphere& sphere2) {
	float distance = Vector3::Length(static_cast<Vector3>(sphere2.center) - sphere1.center);
	if (distance <= sphere1.radius + sphere2.radius) {
		return true;
	}

	return false;
}

bool Collision::SphereToPlane(const Sphere sphere, const Plane& plane) {
	sphere;
	plane;

	if (std::fabs(Vector3::GetDot(sphere.center, plane.normal) - plane.distance) <= sphere.radius) {
		return true;
	}

	return false;
}

bool Collision::LineToPlane(const Line& line, const Plane& plane) {
	if (Vector3::GetDot(plane.normal, line.diff) == 0.0f) {
		return false;
	}

	return true;
}

bool Collision::RayToPlane(const Ray& ray, const Plane& plane) {
	float b = Vector3::GetDot(plane.normal, ray.diff);
	if (b == 0.0f) {
		return false;
	}

	float t = (plane.distance - Vector3::GetDot(ray.origin, plane.normal)) / b;

	if (t >= 0.0f) {
		return true;
	}

	return false;
}

bool Collision::SegmentToPlane(const Segment& segment, const Plane& plane) {
	float b = Vector3::GetDot(plane.normal, segment.diff);
	if (b == 0.0f) {
		return false;
	}

	float t = (plane.distance - Vector3::GetDot(segment.origin, plane.normal)) / b;

	if (t >= 0.0f && t <= 1.0f) {
		return true;
	}

	return false;
}

bool Collision::SegmentToTriangle(const Segment& segment, const Triangle& triangle) {
	Vector3 normal =
		Vector3::GetCross(
			static_cast<Vector3>(triangle.vertices[1]) - static_cast<Vector3>(triangle.vertices[0]),
			static_cast<Vector3>(triangle.vertices[2]) - static_cast<Vector3>(triangle.vertices[1]));

	float distance = Vector3::GetDot(triangle.vertices[0], normal);

	float b = Vector3::GetDot(normal, segment.diff);
	if (b == 0.0f) {
		return false;
	}

	float t = (distance - Vector3::GetDot(segment.origin, normal)) / b;

	if (t < 0.0f || t > 1.0f) {
		return false;
	}

	for (uint32_t index = 0; index < 3; ++index) {
		Vector3 cross =
			Vector3::GetCross(
				static_cast<Vector3>(triangle.vertices[(index + 1) % 3]) - static_cast<Vector3>(triangle.vertices[index]),
				static_cast<Vector3>(triangle.vertices[(index + 1) % 3]) - (static_cast<Vector3>(segment.origin) + (static_cast<Vector3>(segment.diff) * t)));

		if (Vector3::GetDot(cross, normal) > 0.0f) {
			return false;
		}
	}


	return true;
}

bool Collision::AABBToPoint(const AABB& aabb, const Vector3& point) {
	return (
		aabb.min.x <= point.x && aabb.max.x >= point.x &&
		aabb.min.y <= point.y && aabb.max.y >= point.y &&
		aabb.min.z <= point.z && aabb.max.z >= point.z);
}

bool Collision::AABBToAABB(const AABB& aabb1, const AABB& aabb2) {
	return (
		aabb1.min.x <= aabb2.max.x && aabb1.max.x >= aabb2.min.x &&
		aabb1.min.y <= aabb2.max.y && aabb1.max.y >= aabb2.min.y &&
		aabb1.min.z <= aabb2.max.z && aabb1.max.z >= aabb2.min.z);
}

bool Collision::AABBToSphere(const AABB& aabb, const Sphere sphere) {
	Vector3 closestPoint{ Clamp(sphere.center,aabb.min,aabb.max) };

	float distance = Vector3::Length(closestPoint - sphere.center);

	return (distance <= sphere.radius);
}

bool Collision::AABBToSphereFarthest(const AABB& aabb, const Sphere sphere) {
	Vector3 farthestPoint;

	// X軸
	if (std::abs(sphere.center.x - aabb.min.x) >
		std::abs(sphere.center.x - aabb.max.x))
	{
		farthestPoint.x = aabb.min.x;
	} else
	{
		farthestPoint.x = aabb.max.x;
	}

	// Y軸
	if (std::abs(sphere.center.y - aabb.min.y) >
		std::abs(sphere.center.y - aabb.max.y))
	{
		farthestPoint.y = aabb.min.y;
	} else
	{
		farthestPoint.y = aabb.max.y;
	}

	// Z軸
	if (std::abs(sphere.center.z - aabb.min.z) >
		std::abs(sphere.center.z - aabb.max.z))
	{
		farthestPoint.z = aabb.min.z;
	} else
	{
		farthestPoint.z = aabb.max.z;
	}

	float distance = Vector3::Length(farthestPoint - sphere.center);

	return distance <= sphere.radius;
}

bool Collision::AABBToSegment(const AABB& aabb, const Segment& segment) {
	Vector3 minT;
	Vector3 maxT;

	float tMin;
	float tMax;
	Vector3 nearT;
	Vector3 farT;

	minT.x = (aabb.min.x - segment.origin.x) / segment.diff.x;
	maxT.x = (aabb.max.x - segment.origin.x) / segment.diff.x;

	minT.y = (aabb.min.y - segment.origin.y) / segment.diff.y;
	maxT.y = (aabb.max.y - segment.origin.y) / segment.diff.y;

	minT.z = (aabb.min.z - segment.origin.z) / segment.diff.z;
	maxT.z = (aabb.max.z - segment.origin.z) / segment.diff.z;

	nearT.x = std::min(minT.x, maxT.x);
	farT.x = std::max(minT.x, maxT.x);
	nearT.y = std::min(minT.y, maxT.y);
	farT.y = std::max(minT.y, maxT.y);
	nearT.z = std::min(minT.z, maxT.z);
	farT.z = std::max(minT.z, maxT.z);

	tMin = std::max(std::max(nearT.x, nearT.y), nearT.z);
	tMax = std::min(std::min(farT.x, farT.y), farT.z);

	if (tMin <= tMax) {
		if ((maxT.x >= 0.0f && maxT.x <= 1.0f) || (minT.x >= 0.0f && minT.x <= 1.0f) ||
			(maxT.y >= 0.0f && maxT.y <= 1.0f) || (minT.y >= 0.0f && minT.y <= 1.0f) ||
			(maxT.z >= 0.0f && maxT.z <= 1.0f) || (minT.z >= 0.0f && minT.z <= 1.0f)) {
			return true;
		}
	}

	return false;
}

bool Collision::AABBToLine(const AABB& aabb, const Line& line) {
	Vector3 minT;
	Vector3 maxT;

	float tMin;
	float tMax;
	Vector3 nearT;
	Vector3 farT;

	minT.x = (aabb.min.x - line.origin.x) / line.diff.x;
	maxT.x = (aabb.max.x - line.origin.x) / line.diff.x;

	minT.y = (aabb.min.y - line.origin.y) / line.diff.y;
	maxT.y = (aabb.max.y - line.origin.y) / line.diff.y;

	minT.z = (aabb.min.z - line.origin.z) / line.diff.z;
	maxT.z = (aabb.max.z - line.origin.z) / line.diff.z;

	nearT.x = std::min(minT.x, maxT.x);
	farT.x = std::max(minT.x, maxT.x);
	nearT.y = std::min(minT.y, maxT.y);
	farT.y = std::max(minT.y, maxT.y);
	nearT.z = std::min(minT.z, maxT.z);
	farT.z = std::max(minT.z, maxT.z);

	tMin = std::max(std::max(nearT.x, nearT.y), nearT.z);
	tMax = std::min(std::min(farT.x, farT.y), farT.z);

	if (tMin <= tMax) {
		return true;
	}

	return false;
}

bool Collision::AABBToRay(const AABB& aabb, const Ray& ray) {
	Vector3 minT;
	Vector3 maxT;

	float tMin;
	float tMax;
	Vector3 nearT;
	Vector3 farT;

	minT.x = (aabb.min.x - ray.origin.x) / ray.diff.x;
	maxT.x = (aabb.max.x - ray.origin.x) / ray.diff.x;

	minT.y = (aabb.min.y - ray.origin.y) / ray.diff.y;
	maxT.y = (aabb.max.y - ray.origin.y) / ray.diff.y;

	minT.z = (aabb.min.z - ray.origin.z) / ray.diff.z;
	maxT.z = (aabb.max.z - ray.origin.z) / ray.diff.z;

	nearT.x = std::min(minT.x, maxT.x);
	farT.x = std::max(minT.x, maxT.x);
	nearT.y = std::min(minT.y, maxT.y);
	farT.y = std::max(minT.y, maxT.y);
	nearT.z = std::min(minT.z, maxT.z);
	farT.z = std::max(minT.z, maxT.z);

	tMin = std::max(std::max(nearT.x, nearT.y), nearT.z);
	tMax = std::min(std::min(farT.x, farT.y), farT.z);

	if (tMin <= tMax) {
		if ((maxT.x <= 1.0f) || (minT.x <= 1.0f) ||
			(maxT.y <= 1.0f) || (minT.y <= 1.0f) ||
			(maxT.z <= 1.0f) || (minT.z <= 1.0f)) {
			return true;
		}
	}

	return false;
}

bool Collision::OBBToSphere(const OBB& obb, const Sphere& sphere) {
	Matrix4x4 matrix;
	Matrix4x4 rotateMatrix;

	rotateMatrix = rotateMatrix.Identity();
	for (uint32_t i = 0; i < 3; i++) {
		rotateMatrix.matrix[i][0] = obb.orientations[i].x;
		rotateMatrix.matrix[i][1] = obb.orientations[i].y;
		rotateMatrix.matrix[i][2] = obb.orientations[i].z;
	}

	matrix = rotateMatrix;
	matrix = matrix * Matrix4x4::MakeTranslateMatrix(obb.center);

	matrix = matrix.Inverse();

	AABB aabbOBBLocal;
	aabbOBBLocal.min = static_cast<Vector3>(obb.size) * -1.0f;
	aabbOBBLocal.max = obb.size;
	Sphere sphereOBBLocal;
	sphereOBBLocal.center = matrix.MatrixTransform(sphere.center);
	sphereOBBLocal.radius = sphere.radius;

	if (AABBToSphere(aabbOBBLocal, sphereOBBLocal)) {
		return true;
	}

	return false;
}

bool Collision::OBBToSphereFarthest(const OBB& obb, const Sphere& sphere) {
	Matrix4x4 matrix;
	Matrix4x4 rotateMatrix;

	rotateMatrix = rotateMatrix.Identity();
	for (uint32_t i = 0; i < 3; i++) {
		rotateMatrix.matrix[i][0] = obb.orientations[i].x;
		rotateMatrix.matrix[i][1] = obb.orientations[i].y;
		rotateMatrix.matrix[i][2] = obb.orientations[i].z;
	}

	matrix = rotateMatrix;
	matrix = matrix * Matrix4x4::MakeTranslateMatrix(obb.center);

	matrix = matrix.Inverse();

	AABB aabbOBBLocal;
	aabbOBBLocal.min = static_cast<Vector3>(obb.size) * -1.0f;
	aabbOBBLocal.max = obb.size;
	Sphere sphereOBBLocal;
	sphereOBBLocal.center = matrix.MatrixTransform(sphere.center);
	sphereOBBLocal.radius = sphere.radius;

	if (AABBToSphereFarthest(aabbOBBLocal, sphereOBBLocal)) {
		return true;
	}

	return false;
}

bool Collision::OBBToLine(const OBB& obb, const Line& line) {
	Matrix4x4 matrix;
	Matrix4x4 rotateMatrix;

	rotateMatrix = rotateMatrix.Identity();
	for (uint32_t i = 0; i < 3; i++) {
		rotateMatrix.matrix[i][0] = obb.orientations[i].x;
		rotateMatrix.matrix[i][1] = obb.orientations[i].y;
		rotateMatrix.matrix[i][2] = obb.orientations[i].z;
	}

	matrix = rotateMatrix;
	matrix = matrix * Matrix4x4::MakeTranslateMatrix(obb.center);

	matrix = matrix.Inverse();

	AABB aabbOBBLocal;
	aabbOBBLocal.min = static_cast<Vector3>(obb.size) * -1.0f;
	aabbOBBLocal.max = obb.size;

	Line lineOBBLocal;
	lineOBBLocal.origin = matrix.MatrixTransform(line.origin);
	lineOBBLocal.diff = matrix.MatrixTransform((static_cast<Vector3>(line.origin) + line.diff)) - lineOBBLocal.origin;

	if (AABBToLine(aabbOBBLocal, lineOBBLocal)) {
		return true;
	}

	return false;
}

bool Collision::OBBToRay(const OBB& obb, const Ray& ray) {
	Matrix4x4 matrix;
	Matrix4x4 rotateMatrix;

	rotateMatrix = rotateMatrix.Identity();
	for (uint32_t i = 0; i < 3; i++) {
		rotateMatrix.matrix[i][0] = obb.orientations[i].x;
		rotateMatrix.matrix[i][1] = obb.orientations[i].y;
		rotateMatrix.matrix[i][2] = obb.orientations[i].z;
	}

	matrix = rotateMatrix;
	matrix = matrix * Matrix4x4::MakeTranslateMatrix(obb.center);

	matrix = matrix.Inverse();

	AABB aabbOBBLocal;
	aabbOBBLocal.min = static_cast<Vector3>(obb.size) * -1.0f;
	aabbOBBLocal.max = obb.size;

	Ray rayOBBLocal;
	rayOBBLocal.origin = matrix.MatrixTransform(ray.origin);
	rayOBBLocal.diff = matrix.MatrixTransform((static_cast<Vector3>(ray.origin) + ray.diff)) - rayOBBLocal.origin;

	if (AABBToRay(aabbOBBLocal, rayOBBLocal)) {
		return true;
	}

	return false;
}

bool Collision::OBBToSegment(const OBB& obb, const Segment& segment) {
	Matrix4x4 matrix;
	Matrix4x4 rotateMatrix;

	rotateMatrix = rotateMatrix.Identity();
	for (uint32_t i = 0; i < 3; i++) {
		rotateMatrix.matrix[i][0] = obb.orientations[i].x;
		rotateMatrix.matrix[i][1] = obb.orientations[i].y;
		rotateMatrix.matrix[i][2] = obb.orientations[i].z;
	}

	matrix = rotateMatrix;
	matrix = matrix * Matrix4x4::MakeTranslateMatrix(obb.center);

	matrix = matrix.Inverse();

	AABB aabbOBBLocal;
	aabbOBBLocal.min = static_cast<Vector3>(obb.size) * -1.0f;
	aabbOBBLocal.max = obb.size;

	Segment segmentOBBLocal;
	segmentOBBLocal.origin = matrix.MatrixTransform(segment.origin);
	segmentOBBLocal.diff = matrix.MatrixTransform(static_cast<Vector3>(segment.origin) + segment.diff) - segmentOBBLocal.origin;

	if (AABBToSegment(aabbOBBLocal, segmentOBBLocal)) {
		return true;
	}

	return false;
}

bool Collision::OBBToOBB(const OBB& obb1, const OBB& obb2) {
	auto CreateVertices = [](const OBB& obb) {
		std::vector<Vector3> vertices;
		Vector3 ax = static_cast<Vector3>(obb.orientations[0]) * obb.size.x;
		Vector3 ay = static_cast<Vector3>(obb.orientations[1]) * obb.size.y;
		Vector3 az = static_cast<Vector3>(obb.orientations[2]) * obb.size.z;

		vertices.push_back(static_cast<Vector3>(obb.center) - ax - ay - az);
		vertices.push_back(static_cast<Vector3>(obb.center) + ax - ay - az);
		vertices.push_back(static_cast<Vector3>(obb.center) - ax + ay - az);
		vertices.push_back(static_cast<Vector3>(obb.center) + ax + ay - az);

		vertices.push_back(static_cast<Vector3>(obb.center) - ax - ay + az);
		vertices.push_back(static_cast<Vector3>(obb.center) + ax - ay + az);
		vertices.push_back(static_cast<Vector3>(obb.center) - ax + ay + az);
		vertices.push_back(static_cast<Vector3>(obb.center) + ax + ay + az);

		return vertices;
		};

	auto ProjectVertices = [](const std::vector<Vector3>& vertices, const Vector3& axis, float& outMin, float& outMax) {
		float dot = Vector3::GetDot(vertices[0], axis);

		outMin = dot;
		outMax = dot;

		for (size_t i = 1; i < vertices.size(); ++i) {
			float projection = Vector3::GetDot(vertices[i], axis);

			if (projection < outMin) {
				outMin = projection;
			}

			if (projection > outMax) {
				outMax = projection;
			}
		}
		};

	std::vector<Vector3> vertices1 = CreateVertices(obb1);
	std::vector<Vector3> vertices2 = CreateVertices(obb2);


	std::vector<Vector3> axes;

	axes.push_back(obb1.orientations[0]);
	axes.push_back(obb1.orientations[1]);
	axes.push_back(obb1.orientations[2]);

	axes.push_back(obb2.orientations[0]);
	axes.push_back(obb2.orientations[1]);
	axes.push_back(obb2.orientations[2]);

	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			Vector3 axis = Vector3::GetCross(obb1.orientations[i], obb2.orientations[j]);

			if (axis.Length() > 0.0001f) {
				axis.Normalize();
				axes.push_back(axis);
			}
		}
	}

	for (const Vector3& axis : axes) {
		float min1, max1;
		float min2, max2;

		ProjectVertices(vertices1, axis, min1, max1);
		ProjectVertices(vertices2, axis, min2, max2);

		float L1 = max1 - min1;
		float L2 = max2 - min2;
		float sumSpan = L1 + L2;
		float longSpan = std::max(max1, max2) - std::min(min1, min2);

		if (sumSpan < longSpan) {
			return false;
		}
	}
	return true;
}

bool Collision::OBBToPositionY(const OBB& obb, float posY, bool isUp) {
	Matrix4x4 matrix;
	Matrix4x4 rotateMatrix;
	rotateMatrix = rotateMatrix.Identity();

	for (uint32_t i = 0; i < 3; i++) {
		rotateMatrix.matrix[i][0] = obb.orientations[i].x;
		rotateMatrix.matrix[i][1] = obb.orientations[i].y;
		rotateMatrix.matrix[i][2] = obb.orientations[i].z;
	}

	matrix = rotateMatrix;
	matrix = matrix * Matrix4x4::MakeTranslateMatrix(obb.center);
	std::vector<Vector3> vertices;
	vertices.push_back(matrix.MatrixTransform({ -obb.size.x,-obb.size.y,-obb.size.z }));
	vertices.push_back(matrix.MatrixTransform({ -obb.size.x,obb.size.y,-obb.size.z }));
	vertices.push_back(matrix.MatrixTransform({ obb.size.x,-obb.size.y,-obb.size.z }));
	vertices.push_back(matrix.MatrixTransform({ obb.size.x,obb.size.y,-obb.size.z }));

	vertices.push_back(matrix.MatrixTransform({ -obb.size.x,-obb.size.y,obb.size.z }));
	vertices.push_back(matrix.MatrixTransform({ -obb.size.x,obb.size.y,obb.size.z }));
	vertices.push_back(matrix.MatrixTransform({ obb.size.x,-obb.size.y,obb.size.z }));
	vertices.push_back(matrix.MatrixTransform({ obb.size.x,obb.size.y,obb.size.z }));
	for (Vector3 vertex : vertices) {
		if (vertex.x >= posY) {
			if (isUp) {
				return true;
			}
		} else {
			if (!isUp) {
				return true;
			}
		}
	}

	return false;
}

bool Collision::SimpleOBBToTorus(const OBB& obb, const Transform& transform, float majorRadius, float minorRadius) {
	if (OBBToSphere(obb, { transform.GetWorldPosition(),(majorRadius / 2.0f) + (minorRadius / 2.0f) })) {
		if (OBBToSphereFarthest(obb, { transform.GetWorldPosition(),(majorRadius / 2.0f) - (minorRadius / 2.0f) })) {
			return false;
		} else {
			if (OBBToPositionY(obb,transform.GetWorldPosition().y + minorRadius,false) && OBBToPositionY(obb, transform.GetWorldPosition().y - minorRadius,true)) {
				return true;
			}
		}
	}
	return false;
}
