#pragma once

#include <math.h>
#include <assert.h>
#include "Vector2.h"
#include "Vertex.h"

struct Matrix3x3 {
	float matrix[3][3];
};

/// <summary>
/// 3x3行列同士の掛け算
/// </summary>
/// <param name="matrix1">一つ目の行列</param>
/// <param name="matrix2">二つ目の行列</param>
/// <returns></returns>
Matrix3x3 Matrix3x3Multiply(Matrix3x3 matrix1, Matrix3x3 matrix2);

/// <summary>
/// 移動
/// </summary>
/// <param name="vector"></param>
/// <param name="matrix"></param>
/// <returns></returns>
Vector2 Transform(Vector2 vector, Matrix3x3 matrix);

/// <summary>
/// 4頂点の移動
/// </summary>
/// <param name="vertex"></param>
/// <param name="matrix"></param>
/// <returns></returns>
Vertex4 VertexTransform(Vertex4 vertex, Matrix3x3 matrix);

Matrix3x3 MakeTranslateMatrix(Vector2 translate);

Matrix3x3 InverseMatrix3x3(Matrix3x3 matrix);

Matrix3x3 MakeAffineMatrix(Vector2 scale, float rotate, Vector2 translate);

Matrix3x3 MakeOrthographicMatrix(Vertex4 vertex);

Matrix3x3 MakeViewportMatrix(Vector2 leftTop, float width, float height);

Vertex4 MakeCameraVertex(Vertex4 vertex, Matrix3x3 targetMatrix,Matrix3x3 worldMatrix, Vertex4 orthographicVertex, Vector2 leftTop, float width, float height);

Vector2 MakeCameraVector2(Vector2 vector2, Matrix3x3 worldMatrix, Vertex4 orthographicVertex, Vector2 leftTop, float width, float height);