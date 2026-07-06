#pragma once
#include <cmath>

struct Line;

struct Ray;

struct Segment;

class Vector3 {
public:
	float x;
	float y;
	float z;

	Vector3 operator-();

	Vector3 operator+(const Vector3& v1);
	Vector3 operator+(float scalar);
	Vector3 operator-(const Vector3& v1);
	Vector3 operator-(float scalar);
	Vector3 operator*(float scalar);
	Vector3 operator*(const Vector3& v1);
	Vector3 operator/(float scalar);

	Vector3 operator+=(const Vector3& v1);
	Vector3 operator-=(const Vector3& v1);
	Vector3 operator*=(float scalar);
	Vector3 operator*=(const Vector3& v1);
	Vector3 operator/=(float scalar);

	float Dot(const Vector3& v1);
	static float GetDot(const Vector3& v1, const Vector3& v2);

	float Length();
	static float Length(const Vector3 v);

	Vector3 Normalize();
	static Vector3 Normalize(const Vector3& v);

	Vector3 Cross(const Vector3& v1);
	static Vector3 GetCross(const Vector3& v1, const Vector3& v2);

	Vector3 Project(Vector3& v1);
	static Vector3 Project(Vector3 v1, Vector3 v2);

	Vector3 ClosestPoint(Segment segment);

	static Vector3 ClosestPoint(Vector3 origin, Segment segment);

	Vector3 ClosestPoint(Line line);

	static Vector3 ClosestPoint(Vector3 point, Line line);

	Vector3 Perpendicular();

	static Vector3 GetPerpendicular(Vector3 vector);
};

