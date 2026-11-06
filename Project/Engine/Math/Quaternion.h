#pragma once
#include "Vector3.h"
#include "cmath"
class Quaternion{
public:
	float x;
	float y;
	float z;
	float w;
public:
	// 虚部(ベクトル部)を取得する.
	Vector3 GetVector3() const { return {x,y,z}; };

	void SetVector3(const Vector3& vector3) { x = vector3.x; y = vector3.y; z = vector3.z; };

	// 積を求める.
	static Quaternion Multiply(const Quaternion& lhs,const Quaternion& rhs);
	Quaternion operator+(Quaternion r1);
	Quaternion operator*(Quaternion r1);
	Quaternion operator*(float scalar);
	Quaternion operator*=(Quaternion r1);
	Quaternion operator-();

	float Dot(const Quaternion& q1) const;

	// 乗法単位元の値を取得する.
	static Quaternion Identity() { return Quaternion(0.0f, 0.0f, 0.0f, 1.0f); };

	// 共役(虚部(ベクトル部)を反転させた値)を取得する
	Quaternion Conjugation() const;

	float Norm() const { return sqrt(pow(w, 2.0f) + pow(x, 2.0f)+ pow(y, 2.0f) +pow(z, 2.0f)); };
	Quaternion Normalize() const;

	Quaternion Inverse() const;

	static Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis,float angle);

	Vector3 RotateVector(const Vector3& vector);

	static Quaternion GetDirectionToDirection(const Vector3& from, const Vector3& to);
};

