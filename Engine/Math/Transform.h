#pragma once
#include "Vector3.h"
#include "Vector2.h"
#include "Shape.h"

class Matrix4x4;

class Transform{
public:
	enum class StanderdSize {
		kMax,
		kMin,
	};

	void Initialize();

	Matrix4x4 GetAffineMatrix()const;

	Sphere GetSphereMin()const;

	Sphere GetSphereMax()const;
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

