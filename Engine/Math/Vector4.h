#pragma once
#include "Vector3.h"

class Vector4{
public:
	void SetColorWithoutAlpha(Vector3 rgb, float alpha) { x = rgb.x; y = rgb.y; z = rgb.z; w = alpha; };
public:
	float x;
	float y;
	float z;
	float w;
};

