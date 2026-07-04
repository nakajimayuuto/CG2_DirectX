#pragma once
#include "Vector3.h"
#include "Vector2.h"
#include "Shape.h"

class Matrix4x4;

class Transform{
public:
	//enum class StanderdSize {
	//	kMax,
	//	kMin,
	//};

	//Transform(Vector3 newScale, Vector3 newRotate, Vector3 newTranslate) { scale = newScale; rotate = newRotate; translate = newTranslate; };


	void Initialize();
	static Transform GetInitialValue();
	static Transform GetInitialValue(const Vector3& scale, const Vector3& rotate, const Vector3& translate);

	Matrix4x4 GetAffineMatrix()const;

	Matrix4x4 GetScaleMatrix()const;
	Matrix4x4 GetRotateMatrix()const;
	Matrix4x4 GetTranslateMatrix()const;

	Sphere GetSphereMin()const;

	Sphere GetSphereMax()const;

	void TransformSynthesis(const Transform& targetTransform);

	void SetParent(const Transform* parent) { parent_ = parent; };

	const Transform* GetParent() { return parent_; };

	void ClearParent() { parent_ = nullptr; };

	float GetMaxScale()const;
public:
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
private:
	const Transform* parent_ = nullptr;
};

class Transform2D{
public:
	void Initialize();
public:
	Vector2 scale;
	float rotate;
	Vector2 translate;
};

