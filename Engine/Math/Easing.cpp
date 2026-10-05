#include "Easing.h"
#include <assert.h>
#include <algorithm>

float Easing(float before, float after, int time, int timeMax, EaseType type) {
	float x = ((100.0f / timeMax) * (time / 100.0f));
	float easedTime = 0.0f;

	const float c1 = 1.7158f;
	const float c3 = c1 + 1.0f;

	switch (type) {
	case EaseType::kConstant:
		easedTime = x;
		break;
	case EaseType::kEaseIn:
		easedTime = pow(x, 2.0f);
		break;
	case EaseType::kEaseOut:
		easedTime = 1.0f - pow(1.0f - x, 3.0f);
		break;
	case EaseType::kEaseInOut:
		easedTime = -(cos(std::numbers::pi_v<float> * x) - 1.0f) / 2.0f;
		break;
	case EaseType::kEaseInBack:

		easedTime = c3 * pow(x, 3.0f) - c1 * pow(x, 2.0f);
		break;
	case EaseType::kEaseOutBack:
		easedTime = 1.0f + c3 * pow(x - 1.0f, 3.0f) + c1 * pow(x - 1.0f, 2.0f);
		break;
	}

	return (1.0f - easedTime) * before + easedTime * after;
};

float Easing(float before, float after, float time, float timeMax, EaseType type) {
	float x = ((100.0f / timeMax) * (time / 100.0f));
	float easedTime = 0.0f;

	const float c1 = 1.7158f;
	const float c3 = c1 + 1.0f;

	x = std::clamp(x,0.0f,1.0f);

	switch (type) {
	case EaseType::kConstant:
		easedTime = x;
		break;
	case EaseType::kEaseIn:
		easedTime = pow(x, 2.0f);
		break;
	case EaseType::kEaseOut:
		easedTime = 1.0f - pow(1.0f - x, 3.0f);
		break;
	case EaseType::kEaseInOut:
		easedTime = -(cos(std::numbers::pi_v<float> * x) - 1.0f) / 2.0f;
		break;
	case EaseType::kEaseInBack:

		easedTime = c3 * pow(x, 3.0f) - c1 * pow(x, 2.0f);
		break;
	case EaseType::kEaseOutBack:
		easedTime = 1.0f + c3 * pow(x - 1.0f, 3.0f) + c1 * pow(x - 1.0f, 2.0f);
		break;
	}

	return (1.0f - easedTime) * before + easedTime * after;
}

float Lerp(float before, float after, float time) {
	return (1.0f - time) * before + time * after;
}

Vector3 Lerp(Vector3 before, Vector3 after, float time) {
	Vector3 result = before;

	result.x = Lerp(before.x, after.x, time);
	result.y = Lerp(before.y, after.y, time);
	result.z = Lerp(before.z, after.z, time);

	return result;
}

float LerpShortAngle(float before, float after, float time){
	float diff = after - before;
	diff = std::fmod(diff,Radian(360.0f));

	if (diff > Radian(180.0f)) {
		diff = diff - Radian(360.0f);
	} else if (diff < -Radian(180.0f)) {
		diff = diff + Radian(360.0f);
	}



	return before + (diff * time);
}

Vector3 LerpShortAngle(Vector3 before, Vector3 after, float time){
	Vector3 result;
	result.x = LerpShortAngle(before.x,after.x,time);
	result.y = LerpShortAngle(before.y,after.y,time);
	result.z = LerpShortAngle(before.z,after.z,time);

	return result;
}

Vector3 Slerp(Vector3 before, Vector3 after, float time){
	Vector3 result = before;
	
	float theta = before.Dot(after) / (before.Length() * after.Length());

	theta = std::clamp(theta, -1.0f, 1.0f);

	if (theta == 1.0f) {
		return before;
	} else if (theta == -1.0f) {
		theta = -0.5f;
	}

	theta = std::acos(theta);

	auto SlerpFloat = [](float before, float after, float time, float theta) {return ((std::sin((1.0f - time) * theta) / std::sin(theta)) * before) + ((std::sin(time * theta) / std::sin(theta)) * after); };

	result.x = SlerpFloat(before.x, after.x, time,theta);
	result.y = SlerpFloat(before.y, after.y, time,theta);
	result.z = SlerpFloat(before.z, after.z, time,theta);

	return result;
}

int Easing(int before, int after, float time, float timeMax, EaseType type){
	return static_cast<int>(Easing(static_cast<float>(before), static_cast<float>(after), time, timeMax, type));
}

Vector2 Easing(Vector2 before, Vector2 after, float time, float timeMax, EaseType type){
	Vector2 result;

	result.x = Easing(before.x, after.x, time, timeMax, type);
	result.y = Easing(before.y, after.y, time, timeMax, type);

	return result;
};

Vector3 Easing(Vector3 before, Vector3 after, float time, float timeMax, EaseType type){
	Vector3 result;

	result.x = Easing(before.x, after.x, time, timeMax, type);
	result.y = Easing(before.y, after.y, time, timeMax, type);
	result.z = Easing(before.z, after.z, time, timeMax, type);

	return result;
};

Vector2 Bezier(const Vector2& p0, const Vector2& p1, const Vector2& p2, int t,int maxT) {
	Vector2 p0p1;
	Vector2 p1p2;
	Vector2 p;
	p0p1.x = Easing(p0.x, p1.x, t, maxT, EaseType::kConstant);
	p0p1.y = Easing(p0.y, p1.y, t, maxT, EaseType::kConstant);
	p1p2.x = Easing(p1.x, p2.x, t, maxT, EaseType::kConstant);
	p1p2.y = Easing(p1.y, p2.y, t, maxT, EaseType::kConstant);
	p.x = Easing(p0p1.x, p1p2.x, t, maxT, EaseType::kConstant);
	p.y = Easing(p0p1.y, p1p2.y, t, maxT, EaseType::kConstant);

	return p;
}

Vector3 CatmullRomInterpolation(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float time, float maxTime){
	float t = ((100.0f / maxTime) * (time / 100.0f));

	float t2 = t * t;
	float t3 = t2 * t;

	Vector3 e[4];
	e[3] = (static_cast<Vector3>(p0) * -1.0f) + (static_cast<Vector3>(p1) * 3.0f) + (static_cast<Vector3>(p2) * -3.0f) + p3;
	e[2] = (static_cast<Vector3>(p0) * 2.0f) + (static_cast<Vector3>(p1) * -5.0f) + (static_cast<Vector3>(p2) * 4.0f) - p3;
	e[1] = (static_cast<Vector3>(p0) * -1.0f) + p2;
	e[0] = (static_cast<Vector3>(p1) * 2.0f);

	return ((e[3] * t3) + (e[2] * t2) + (e[1] * t) + e[0]) / 2.0f;
}

Vector3 CatmullRomInterpolation(const std::vector<Vector3>& point, float time, float maxTime){
	assert(point.size() >= 4 && "制御点は4点以上必要です");

	size_t division = point.size() - 1;
	float areaWidth = 1.0f / static_cast<float>(division);
	float t = ((100.0f / maxTime) * (time / 100.0f));

	float t_2 = std::fmod(t,areaWidth) * static_cast<float>(division);

	t_2 = std::clamp(t_2,0.0f,1.0f);

	size_t index = static_cast<size_t>(t / areaWidth);
	index = std::clamp(index, static_cast<size_t>(0),point.size() - 2);

	size_t index0 = index - 1;
	size_t index1 = index;
	size_t index2 = index + 1;
	size_t index3 = index + 2;

	if (index == 0) {
		index0 = index1;
	}

	if (index3 == point.size()) {
		index3 = index2;
	}

	const Vector3& p0 = point[index0];
	const Vector3& p1 = point[index1];
	const Vector3& p2 = point[index2];
	const Vector3& p3 = point[index3];

	return CatmullRomInterpolation(p0,p1,p2,p3,t_2,1.0f);
}
