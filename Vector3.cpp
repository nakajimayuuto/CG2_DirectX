#include "Vector3.h"

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

float Vector3::Dot(const Vector3& v1){
	return (x * v1.x) + (y * v1.y) + (z * v1.z);
}

float Vector3::Length() {
	return sqrt(pow(x, 2.0f) + pow(y, 2.0f) + pow(z, 2.0f));
}

Vector3 Vector3::Normalize(){
	float length = Length();
	Vector3 normalize;

	normalize.x = 0.0f;
	normalize.y = 0.0f;
	normalize.z = 0.0f;

	if (length != 0.0f) {
		normalize.x = x / length;
		normalize.y = y / length;
		normalize.z = z / length;
	}

	return normalize;
}

Vector3 Vector3::Cross(const Vector3& v1){
	return { (y * v1.z) - (z * v1.y) ,(z * v1.x) - (x * v1.z),(x * v1.y) - (y * v1.x)};
}
