#include "Vector3.h"
#include "Shape.h"
#include <algorithm>
#include "Math.h"

using namespace std;

Vector3 Vector3::operator+(const Vector3& v1){
	Vector3 result;
	result.x = x + v1.x;
	result.y = y + v1.y;
	result.z = z + v1.z;
	return result;
}

Vector3 Vector3::operator-(const Vector3& v1){
	Vector3 result;
	result.x = x - v1.x;
	result.y = y - v1.y;
	result.z = z - v1.z;
	return result;
}

Vector3 Vector3::operator*(float scalar){
	Vector3 result;
	result.x = x * scalar;
	result.y = y * scalar;
	result.z = z * scalar;
	return result;
}

Vector3 Vector3::operator*(const Vector3& v1){
	Vector3 result;
	result.x = x * v1.x;
	result.y = y * v1.y;
	result.z = z * v1.z;
	return result;
}

float Vector3::Dot(const Vector3& v1) {
	return (x * v1.x) + (y * v1.y) + (z * v1.z);
}

float Vector3::GetDot(const Vector3& v1, const Vector3& v2) {
	return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}

float Vector3::Length() {
	return sqrt(pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f));
}

float Vector3::Length(const Vector3 v) {
	return sqrt(pow(v.x, 2.0f) + pow(v.y, 2.0f) + pow(v.z, 2.0f));
}

Vector3 Vector3::Normalize() {
	float length = Length();
	Vector3 result;

	result.x = 0.0f;
	result.y = 0.0f;
	result.z = 0.0f;

	if (length != 0.0f) {
		result.x = x / length;
		result.y = y / length;
		result.z = z / length;
	}

	return result;
}

Vector3 Vector3::Normalize(const Vector3& v) {
	float length = Length(v);
	Vector3 result;

	result.x = 0.0f;
	result.y = 0.0f;
	result.z = 0.0f;

	if (length != 0.0f) {
		result.x = v.x / length;
		result.y = v.y / length;
		result.z = v.z / length;
	}

	return result;
}

Vector3 Vector3::Cross(const Vector3& v1) {
	return { (y * v1.z) - (z * v1.y) ,(z * v1.x) - (x * v1.z),(x * v1.y) - (y * v1.x) };
}

Vector3 Vector3::GetCross(const Vector3& v1, const Vector3& v2) {
	return { (v1.y * v2.z) - (v1.z * v2.y) ,(v1.z * v2.x) - (v1.x * v2.z),(v1.x * v2.y) - (v1.y * v2.x) };
}

Vector3 Vector3::Project(Vector3& v1) {
	return (Vector3(x, y, z) * v1.Normalize()) * v1.Normalize();
}

Vector3 Vector3::Project(Vector3 v1, Vector3 v2) {
	return v2.Normalize() * v1.Dot(v2.Normalize());
}

Vector3 Vector3::ClosestPoint(Segment segment) {
	Vector3 point;
	point = Vector3(x, y, z);
	Vector3 result = segment.origin + Project((point - segment.origin), segment.diff);
	return result;
}

Vector3 Vector3::ClosestPoint(Vector3 point, Segment segment) {
	return segment.origin + Project((point - segment.origin), segment.diff);
}

Vector3 Vector3::ClosestPoint(Line line) {
	Vector3 point;
	point = Vector3(x, y, z);
	Vector3 result = line.origin + Project((point - line.origin), line.diff);
	return result;
}

Vector3 Vector3::ClosestPoint(Vector3 point, Line line) {
	return line.origin + Project((point - line.origin), line.diff);
}

Vector3 Vector3::Perpendicular() {
	if (x != 0.0f || y != 0.0f) {
		return{ -y,x,0.0f };
	}

	return{ 0.0f,-z,y };
}

Vector3 Vector3::GetPerpendicular(Vector3 vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return{ -vector.y,vector.x,0.0f };
	}

	return{ 0.0f,-vector.z,vector.y };
}
