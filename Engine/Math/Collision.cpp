#define NOMINMAX
#include "Collision.h"
#include <cmath>
#include "Math.h"
#include <algorithm>

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

bool Collision::AABBToPoint(const AABB& aabb, const Vector3& point){
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
	obb1;
	obb2;
	//Matrix4x4 matrix1;
	//Matrix4x4 rotateMatrix;
	//Matrix4x4 matrix2;
	//
	//Line lineObb1[3];
	//Line lineObb2[3];
	//
	//rotateMatrix = rotateMatrix.Identity();
	//for (uint32_t i = 0; i < 3; i++) {
	//	rotateMatrix.matrix[i][0] = obb1.orientations[i].x;
	//	rotateMatrix.matrix[i][1] = obb1.orientations[i].y;
	//	rotateMatrix.matrix[i][2] = obb1.orientations[i].z;
	//}
	//
	//matrix1 = rotateMatrix;
	//matrix1 = matrix1 * Matrix4x4::MakeTranslateMatrix(obb1.center);
	//
	//rotateMatrix = rotateMatrix.Identity();
	//for (uint32_t i = 0; i < 3; i++) {
	//	rotateMatrix.matrix[i][0] = obb2.orientations[i].x;
	//	rotateMatrix.matrix[i][1] = obb2.orientations[i].y;
	//	rotateMatrix.matrix[i][2] = obb2.orientations[i].z;
	//}
	//
	//matrix2 = rotateMatrix;
	//matrix2 = matrix2 * Matrix4x4::MakeTranslateMatrix(obb2.center);
	//
	//lineObb1[0].origin = matrix1.Transform({ 0.0f,0.0f,0.0f });
	//lineObb1[1].origin = matrix1.Transform({ 0.0f,0.0f,0.0f });
	//lineObb1[2].origin = matrix1.Transform({ 0.0f,0.0f,0.0f });
	//Vector3 sizeObb1 = matrix1.Transform(obb1.size);
	//lineObb1[0].diff = matrix1.Transform({ obb1.size.x,0.0f,0.0f });
	//lineObb1[1].diff = matrix1.Transform({ 0.0f,obb1.size.y,0.0f });
	//lineObb1[2].diff = matrix1.Transform({ 0.0f,0.0f,obb1.size.z });
	//
	//lineObb2[0].origin = matrix2.Transform({ 0.0f,0.0f,0.0f });
	//lineObb2[1].origin = matrix2.Transform({ 0.0f,0.0f,0.0f });
	//lineObb2[2].origin = matrix2.Transform({ 0.0f,0.0f,0.0f });
	//Vector3 sizeObb2 = matrix2.Transform(obb2.size);
	//lineObb2[0].diff = matrix2.Transform({ obb2.size.x,0.0f,0.0f });
	//lineObb2[1].diff = matrix2.Transform({ 0.0f,obb2.size.y,0.0f });
	//lineObb2[2].diff = matrix2.Transform({ 0.0f,0.0f,obb2.size.z });
	//
	//Vector3 obb1Vertices[8];
	//Vector3 obb2Vertices[8];
	//
	//Vector3 obb1ClosestPoint[3][8];
	//Vector3 obb2ClosestPoint[3][8];
	//
	//obb1Vertices[0] = matrix2.Transform({ -obb1.size.x,-obb1.size.y,-obb1.size.z });
	//obb1Vertices[1] = matrix2.Transform({ obb1.size.x,-obb1.size.y,-obb1.size.z });
	//obb1Vertices[2] = matrix2.Transform({ -obb1.size.x,obb1.size.y,-obb1.size.z });
	//obb1Vertices[3] = matrix2.Transform({ -obb1.size.x,-obb1.size.y,obb1.size.z });
	//obb1Vertices[4] = matrix2.Transform({ obb1.size.x,obb1.size.y,-obb1.size.z });
	//obb1Vertices[5] = matrix2.Transform({ obb1.size.x,-obb1.size.y,obb1.size.z });
	//obb1Vertices[6] = matrix2.Transform({ -obb1.size.x,obb1.size.y,obb1.size.z });
	//obb1Vertices[7] = matrix2.Transform({ obb1.size.x,obb1.size.y,obb1.size.z });
	//
	//obb2Vertices[0] = matrix2.Transform({ -obb2.size.x,-obb2.size.y,-obb2.size.z });
	//obb2Vertices[1] = matrix2.Transform({ obb2.size.x,-obb2.size.y,-obb2.size.z });
	//obb2Vertices[2] = matrix2.Transform({ -obb2.size.x,obb2.size.y,-obb2.size.z });
	//obb2Vertices[3] = matrix2.Transform({ -obb2.size.x,-obb2.size.y,obb2.size.z });
	//obb2Vertices[4] = matrix2.Transform({ obb2.size.x,obb2.size.y,-obb2.size.z });
	//obb2Vertices[5] = matrix2.Transform({ obb2.size.x,-obb2.size.y,obb2.size.z });
	//obb2Vertices[6] = matrix2.Transform({ -obb2.size.x,obb2.size.y,obb2.size.z });
	//obb2Vertices[7] = matrix2.Transform({ obb2.size.x,obb2.size.y,obb2.size.z });
	//
	//float max1[3];
	//float min1[3];
	//
	//float max2[3];
	//float min2[3];
	//
	//for (uint32_t i = 0; i < 3; i++) {
	//	for (uint32_t j = 0; j < 8; j++) {
	//		obb1ClosestPoint[i][j] = obb1Vertices[j].ClosestPoint(lineObb1[i]);
	//		obb2ClosestPoint[i][j] = obb2Vertices[j].ClosestPoint(lineObb2[i]);
	//
	//		if (j == 0) {
	//			min1[i] = std::min(std::min(obb1ClosestPoint[i][j].x, obb1ClosestPoint[i][j].y), obb1ClosestPoint[i][j].z);
	//			max1[i] = std::max(std::max(obb1ClosestPoint[i][j].x, obb1ClosestPoint[i][j].y), obb1ClosestPoint[i][j].z);
	//			min2[i] = std::min(std::min(obb2ClosestPoint[i][j].x, obb2ClosestPoint[i][j].y), obb2ClosestPoint[i][j].z);
	//			max2[i] = std::max(std::max(obb2ClosestPoint[i][j].x, obb2ClosestPoint[i][j].y), obb2ClosestPoint[i][j].z);
	//		} else {
	//			float tempMin = std::min(std::min(obb1ClosestPoint[i][j].x, obb1ClosestPoint[i][j].y), obb1ClosestPoint[i][j].z);
	//			float tempMax = std::max(std::max(obb1ClosestPoint[i][j].x, obb1ClosestPoint[i][j].y), obb1ClosestPoint[i][j].z);
	//
	//			min1[i] = std::min(min1[i], tempMin);
	//			max1[i] = std::max(max1[i], tempMax);
	//
	//			tempMin = std::min(std::min(obb2ClosestPoint[i][j].x, obb2ClosestPoint[i][j].y), obb2ClosestPoint[i][j].z);
	//			tempMax = std::max(std::max(obb2ClosestPoint[i][j].x, obb2ClosestPoint[i][j].y), obb2ClosestPoint[i][j].z);
	//
	//			min2[i] = std::min(min2[i], tempMin);
	//			max2[i] = std::max(max2[i], tempMax);
	//		}
	//	}
	//}
	//
	//float L1;
	//float L2;
	//
	//float sumSpan;
	//float longSpan;
	//
	//for (uint32_t i = 0; i < 3; i++) {
	//	L1 = max1[i] - min1[i];
	//	L2 = max2[i] - min2[i];
	//
	//	sumSpan = L1 + L2;
	//	longSpan = std::max(max1[i], max2[i]) - std::min(min1[i],min2[i]);
	//
	//	if (sumSpan < longSpan) {
			return false;
	//	}
	//}
	//
	//return true;
}
