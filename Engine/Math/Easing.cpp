#include "Easing.h"

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