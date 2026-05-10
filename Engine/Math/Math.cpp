#include "Math.h"

float Length(Vector2 vector2) {
	float length = sqrt(pow(vector2.x, 2.0f) + pow(vector2.y, 2.0f));

	return length;
};


Vector2 Normalize(Vector2 vector2) {
	float length = Length(vector2);
	Vector2 normalize;

	normalize.x = 0.0f;
	normalize.y = 0.0f;

	if (length != 0.0f) {
		normalize.x = vector2.x / length;
		normalize.y = vector2.y / length;
	}

	return normalize;
};


float Radian(float degree) {

	float radian = degree * (std::numbers::pi_v<float> / 180.0f);

	return radian;
}


float Degree(float radian) {

	float degree = radian * (180.0f / std::numbers::pi_v<float>);

	return degree;
}


Vector3 Radian(Vector3 degree) {
	Vector3 radian;

	radian.x = Radian(degree.x);
	radian.y = Radian(degree.y);
	radian.z = Radian(degree.z);

	return radian;
}


Vector3 Degree(Vector3 radian) {
	Vector3 degree;

	degree.x = Degree(radian.x);
	degree.y = Degree(radian.y);
	degree.z = Degree(radian.z);

	return degree;
}

float Clamp(float clamping, float min, float max) {
	float clamp = clamping;

	if (clamping <= min) {
		clamp = min;
	} else if (clamp >= max) {
		clamp = max;
	}

	return clamp;
};

Vector3 Clamp(Vector3 clamping, Vector3 min, Vector3 max) {
	Vector3 result;
	result.x = Clamp(clamping.x, min.x, max.x);
	result.y = Clamp(clamping.y, min.y, max.y);
	result.z = Clamp(clamping.z, min.z, max.z);

	return result;
};

Vector2 Component(Vector2 vector2Start, Vector2 vector2End) {

	float componentX = vector2End.x - vector2Start.x;
	float componentY = vector2End.y - vector2Start.y;

	Vector2 component = { componentX,componentY };

	return component;
}


float DotProduct(Vector2 vector2to1, Vector2 vector2to2) {
	float dot = (vector2to1.x * vector2to2.x) + (vector2to1.y * vector2to2.y);
	return dot;
};


float CrossProduct(Vector2 vector2v1, Vector2 vector2v2) {

	float cross = (vector2v1.x * vector2v2.y) - (vector2v1.y * vector2v2.x);

	return cross;
}

Vector2 Rotate(Vector2 pos, Vector2 centerPos, float theta) {

	Vector2 newPos;
	newPos.x = (pos.x * cos(Radian(theta))) - pos.y * sin(Radian(theta)) + centerPos.x;
	newPos.y = (pos.x * sin(Radian(theta))) - pos.y * cos(Radian(theta)) + centerPos.y;

	return newPos;
}

float Rotate(float pos, float centerPos, float theta) {
	return (pos * cos(Radian(theta))) - pos * sin(Radian(theta)) + centerPos;
}

float VectorToRadian(Vector2 vector) {
	float lengthV1 = Length(vector);
	float lengthV2 = Length({ 1.0f,0.0f });
	float dotProduct = DotProduct(vector, { 1.0f,0.0f });

	if (vector.y < 0.0f) {
		return -acos(dotProduct / (lengthV1 * lengthV2));
	}

	return acos(dotProduct / (lengthV1 * lengthV2));
}

Vector2 RadianToVector(float radian) {

	Vector2 vector2 = {
		cos(Degree(radian) * std::numbers::pi_v<float> / 180.0f),
		sin(Degree(radian) * std::numbers::pi_v<float> / 180.0f)
	};

	return Normalize(vector2);
}



float VectorToDegree(Vector2 vector) {
	float lengthV1 = Length(vector);
	float lengthV2 = Length({ 1.0f,0.0f });
	float dotProduct = DotProduct(vector, { 1.0f,0.0f });

	if (vector.y < 0.0f) {
		return -Degree(acos(dotProduct / (lengthV1 * lengthV2)));
	}

	return Degree(acos(dotProduct / (lengthV1 * lengthV2)));
}

Vector2 DegreeToVector(float degree) {

	Vector2 vector2 = {
		cos(degree * std::numbers::pi_v<float> / 180.0f),
		sin(degree * std::numbers::pi_v<float> / 180.0f)
	};

	return Normalize(vector2);
}