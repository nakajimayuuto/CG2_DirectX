#pragma once
#include "Vector3.h"
#include "Vertex.h"
#include "assert.h"
#include <cmath>

class Transform;

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
	
	Vector3 TransformNomal(const Vector3& vector);

	static Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
	static Matrix4x4 MakeScaleMatrix(const Vector3& scale);
	static Matrix4x4 MakeRotateXMatrix(float radian);
	static Matrix4x4 MakeRotateYMatrix(float radian);
	static Matrix4x4 MakeRotateZMatrix(float radian);
	static Matrix4x4 MakeRotateMatrix(Vector3 rotate);

	Vector3 GetXAxis() { return { matrix[0][0], matrix[1][0], matrix[2][0] }; };
	static Vector3 GetXAxis(const Matrix4x4& matrix) { return { matrix.matrix[0][0], matrix.matrix[1][0], matrix.matrix[2][0] }; };

	Vector3 GetYAxis() { return { matrix[0][1], matrix[1][1], matrix[2][1] }; };
	static Vector3 GetYAxis(const Matrix4x4& matrix) { return { matrix.matrix[0][1], matrix.matrix[1][1], matrix.matrix[2][1] }; };

	Vector3 GetZAxis() { return { matrix[0][2], matrix[1][2], matrix[2][2] }; };
	static Vector3 GetZAxis(const Matrix4x4& matrix) { return { matrix.matrix[0][2], matrix.matrix[1][2], matrix.matrix[2][2] }; };

	static Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
	static Matrix4x4 MakeAffineMatrix(const Transform& transform);
	Transform GetMatrixToTransform();
	Vector3 GetMatrixToScale();
	Vector3 GetMatrixToRotate();
	Vector3 GetMatrixToTranslate();

	static Matrix4x4 MakeOrthographicMatrix(Vertex4 vertex4, float zNear, float zFar);
	static Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
	static Matrix4x4 MakeViewportMatrix(Vector3 leftTop, float width, float height, float minDepth, float maxDepth);
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Matrix4x4 WorldInverseTranspose;
};
