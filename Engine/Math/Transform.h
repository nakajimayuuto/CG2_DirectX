#pragma once
#include "Vector3.h"
#include "Vector2.h"

class Transform{
public:
	void Initialize();
public:
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

class Transform2D{
public:
	void Initialize();
public:
	Vector2 scale;
	float rotate;
	Vector2 translate;
};

