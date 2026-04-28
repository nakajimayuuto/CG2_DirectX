#pragma once
#include "Vector3.h"
#include "Vertex.h"
#include "Transform.h"
#include "assert.h"
#include <cmath>

class Matrix4x4 {
public:
	float matrix[4][4];

	Matrix4x4 operator+(const Matrix4x4& m1);
	Matrix4x4 operator-(const Matrix4x4& m1);
	Matrix4x4 operator*(float scalar);
	Matrix4x4 operator*(const Matrix4x4& m1);

	Matrix4x4& operator+=(const Matrix4x4& m1);
	Matrix4x4& operator-=(const Matrix4x4& m1);
	Matrix4x4& operator*=(float scalar);
	Matrix4x4& operator*=(const Matrix4x4& m1);

	Matrix4x4 Inverse();
	static Matrix4x4 GetInverse(Matrix4x4 matrix);
	Matrix4x4 Transpose();
	static Matrix4x4 GetTranspose(Matrix4x4 matrix);
	static Matrix4x4 Identity();
	Vector3 MatrixTransform(const Vector3& vector);

	static Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
	static Matrix4x4 MakeScaleMatrix(const Vector3& scale);
	static Matrix4x4 MakeRotateXMatrix(float radian);
	static Matrix4x4 MakeRotateYMatrix(float radian);
	static Matrix4x4 MakeRotateZMatrix(float radian);

	static Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
	static Matrix4x4 MakeAffineMatrix(const Transform& transform);

	static Matrix4x4 MakeOrthographicMatrix(Vertex4 vertex4, float zNear, float zFar);
	static Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
	static Matrix4x4 MakeViewportMatrix(Vector3 leftTop, float width, float height, float minDepth, float maxDepth);
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};
