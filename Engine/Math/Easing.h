#pragma once
#include "Math.h"
#include "Vector2.h"
#include "Vector3.h"
#include <vector>

enum class EaseType {
	kConstant,
	kEaseIn,
	kEaseOut,
	kEaseInOut,
	kEaseInBack,
	kEaseOutBack,
};

float Lerp(float before, float after, float time);

Vector3 Lerp(Vector3 before, Vector3 after, float time);

float LerpShortAngle(float before, float after,float time);

Vector3 LerpShortAngle(Vector3 before, Vector3 after,float time);

Vector3 Slerp(Vector3 before, Vector3 after, float time);

float Easing(float before, float after, int time, int timeMax, EaseType type);

float Easing(float before, float after, float time, float timeMax, EaseType type);

int Easing(int before, int after, float time, float timeMax, EaseType type);

Vector2 Easing(Vector2 before, Vector2 after, float time, float timeMax, EaseType type);

Vector3 Easing(Vector3 before, Vector3 after, float time, float timeMax, EaseType type);

Vector2 Bezier(const Vector2& p0, const Vector2& p1, const Vector2& p2, int t, int maxT);

Vector3 CatmullRomInterpolation(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float time, float maxTime);
Vector3 CatmullRomInterpolation(const std::vector<Vector3>& point, float time, float maxTime);