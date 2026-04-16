#pragma once
#include <cmath>

class Vector3 {
public:
	float x;
	float y;
	float z;

	Vector3 operator+(const Vector3& v1);
	Vector3 operator-(const Vector3& v1);
	Vector3 operator*(float scalar);
	Vector3 operator*(const Vector3& v1);

	float Dot(const Vector3& v1);

	float Length();

	Vector3 Normalize();

	Vector3 Cross(const Vector3& v1);
};

