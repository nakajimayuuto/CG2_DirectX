#pragma once
class Vector2{
public:
	float x;
	float y;

	Vector2 operator+(const float& targetFloat);
	Vector2 operator-(const float& targetFloat);
	Vector2 operator*(const float& targetFloat);
	Vector2 operator/(const float& targetFloat);
	Vector2 operator=(const float& targetFloat);
	Vector2& operator+=(const float& targetFloat);
	Vector2& operator-=(const float& targetFloat);
	Vector2& operator*=(const float& targetFloat);
	Vector2& operator/=(const float& targetFloat);

	Vector2 operator+(const Vector2& targetVector2);
	Vector2 operator-(const Vector2& targetVector2);
	Vector2 operator*(const Vector2& targetVector2);
	Vector2 operator/(const Vector2& targetVector2);
	Vector2& operator+=(const Vector2& targetVector2);
	Vector2& operator-=(const Vector2& targetVector2);
	Vector2& operator*=(const Vector2& targetVector2);
	Vector2& operator/=(const Vector2& targetVector2);
};