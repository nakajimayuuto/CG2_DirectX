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


	/// <summary>
	/// ベクトルの長さを取得する
	/// </summary>
	/// <param name="vector2">長さを取得したいベクトル</param>
	/// <returns></returns>
	float Length();

	/// <summary>
	/// 正規化されたベクトルを取得する
	/// </summary>
	/// <param name="vector2">正規化したいベクトル</param>
	/// <returns></returns>
	Vector2 Normalize();
};