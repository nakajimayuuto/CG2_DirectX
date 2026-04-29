#include "Matrix3x3.h"

/// <summary>
/// 3次元行列の掛け算
/// </summary>
/// <param name="matrix1">1つ目の行列</param>
/// <param name="matrix2">2つ目の行列</param>
/// <returns></returns>
Matrix3x3 Matrix3x3Multiply(Matrix3x3 matrix1, Matrix3x3 matrix2) {
	Matrix3x3 result;
	for (int row = 0; row < 3; row++) {
		for (int column = 0; column < 3; column++) {
			result.matrix[row][column] = matrix1.matrix[row][0] * matrix2.matrix[0][column] + matrix1.matrix[row][1] * matrix2.matrix[1][column] + matrix1.matrix[row][2] * matrix2.matrix[2][column];
		}
	}

	return result;
}

Vector2 Transform(Vector2 vector, Matrix3x3 matrix) {
	Vector2 result;
	result.x = vector.x * matrix.matrix[0][0] + vector.y * matrix.matrix[1][0] + 1.0f * matrix.matrix[2][0];
	result.y = vector.x * matrix.matrix[0][1] + vector.y * matrix.matrix[1][1] + 1.0f * matrix.matrix[2][1];
	float w = vector.x * matrix.matrix[0][2] + vector.y * matrix.matrix[1][2] + 1.0f * matrix.matrix[2][2];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	return result;
}

Vertex4 VertexTransform(Vertex4 vertex, Matrix3x3 matrix) {
	Vertex4 result;

	result.leftTop = Transform(vertex.leftTop, matrix);
	result.rightTop = Transform(vertex.rightTop, matrix);
	result.leftBottom = Transform(vertex.leftBottom, matrix);
	result.rightBottom = Transform(vertex.rightBottom, matrix);

	return result;
}

Matrix3x3 MakeRotateMatrix(float theta) {
	Matrix3x3 result;
	result.matrix[0][0] = cosf(theta);
	result.matrix[0][1] = sinf(theta);
	result.matrix[0][2] = 0.0f;
	result.matrix[1][0] = -sinf(theta);
	result.matrix[1][1] = cosf(theta);
	result.matrix[1][2] = 0.0f;
	result.matrix[2][0] = 0.0f;
	result.matrix[2][1] = 0.0f;
	result.matrix[2][2] = 1.0f;

	return result;
}

Matrix3x3 MakeScaleMatrix(Vector2 scale) {
	Matrix3x3 result;
	result.matrix[0][0] = scale.x;
	result.matrix[0][1] = 0.0f;
	result.matrix[0][2] = 0.0f;
	result.matrix[1][0] = 0.0f;
	result.matrix[1][1] = scale.y;
	result.matrix[1][2] = 0.0f;
	result.matrix[2][0] = 0.0f;
	result.matrix[2][1] = 0.0f;
	result.matrix[2][2] = 1.0f;

	return result;
}

Matrix3x3 MakeTranslateMatrix(Vector2 translate) {
	Matrix3x3 result;
	result.matrix[0][0] = 1.0f;
	result.matrix[0][1] = 0.0f;
	result.matrix[0][2] = 0.0f;
	result.matrix[1][0] = 0.0f;
	result.matrix[1][1] = 1.0f;
	result.matrix[1][2] = 0.0f;
	result.matrix[2][0] = translate.x;
	result.matrix[2][1] = translate.y;
	result.matrix[2][2] = 1.0f;

	return result;
}

Matrix3x3 InverseMatrix3x3(Matrix3x3 matrix) {
	Matrix3x3 result;
	float determinat = 
		(matrix.matrix[0][0] * matrix.matrix[1][1] * matrix.matrix[2][2]) +
		(matrix.matrix[0][1] * matrix.matrix[1][2] * matrix.matrix[2][0]) +
		(matrix.matrix[0][2] * matrix.matrix[1][0] * matrix.matrix[2][1]) -
		(matrix.matrix[0][2] * matrix.matrix[1][1] * matrix.matrix[2][0]) -
		(matrix.matrix[0][1] * matrix.matrix[1][0] * matrix.matrix[2][2]) -
		(matrix.matrix[0][0] * matrix.matrix[1][2] * matrix.matrix[2][1]);

	result.matrix[0][0] = (matrix.matrix[1][1] * matrix.matrix[2][2]) - (matrix.matrix[1][2] * matrix.matrix[2][1]);
	result.matrix[0][1] = -((matrix.matrix[0][1] * matrix.matrix[2][2]) - (matrix.matrix[0][2] * matrix.matrix[2][1]));
	result.matrix[0][2] = (matrix.matrix[0][1] * matrix.matrix[1][2]) - (matrix.matrix[0][2] * matrix.matrix[1][1]);
	result.matrix[1][0] = -((matrix.matrix[1][0] * matrix.matrix[2][2]) - (matrix.matrix[1][2] * matrix.matrix[2][0]));
	result.matrix[1][1] = (matrix.matrix[0][0] * matrix.matrix[2][2]) - (matrix.matrix[0][2] * matrix.matrix[2][0]);
	result.matrix[1][2] = -((matrix.matrix[0][0] * matrix.matrix[1][2]) - (matrix.matrix[0][2] * matrix.matrix[1][0]));
	result.matrix[2][0] = (matrix.matrix[1][0] * matrix.matrix[2][1]) - (matrix.matrix[1][1] * matrix.matrix[2][0]);
	result.matrix[2][1] = -((matrix.matrix[0][0] * matrix.matrix[2][1]) - (matrix.matrix[0][1] * matrix.matrix[2][0]));
	result.matrix[2][2] = (matrix.matrix[0][0] * matrix.matrix[1][1]) - (matrix.matrix[0][1] * matrix.matrix[1][0]);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			result.matrix[i][j] = result.matrix[i][j] / determinat;
		}
	}

	return result;
}

Matrix3x3 MakeAffineMatrix(Vector2 scale, float rotate, Vector2 translate) {
	Matrix3x3 result;
	result.matrix[0][0] = scale.x * cosf(rotate);
	result.matrix[0][1] = scale.x * sinf(rotate);
	result.matrix[0][2] = 0.0f;
	result.matrix[1][0] = scale.y * -sinf(rotate);
	result.matrix[1][1] = scale.y * cosf(rotate);
	result.matrix[1][2] = 0.0f;
	result.matrix[2][0] = translate.x;
	result.matrix[2][1] = translate.y;
	result.matrix[2][2] = 1.0f;

	return result;
}

Matrix3x3 MakeOrthographicMatrix(Vertex4 vertex) {
	Matrix3x3 result;

	for (int row = 0; row < 3; row++) {
		for (int column = 0; column < 3; column++) {
			result.matrix[row][column] = 0.0f;
		}
	}

	result.matrix[0][0] = 2.0f / (vertex.rightBottom.x - vertex.leftTop.x);
	result.matrix[1][1] = 2.0f / (vertex.leftTop.y - vertex.rightBottom.y);

	result.matrix[2][0] = (vertex.leftTop.x + vertex.rightBottom.x)/ (vertex.leftTop.x - vertex.rightBottom.x);
	result.matrix[2][1] = (vertex.leftTop.y + vertex.rightBottom.y) / (vertex.rightBottom.y - vertex.leftTop.y);
	result.matrix[2][2] = 1.0f;

	return result;
}

Matrix3x3 MakeViewportMatrix(Vector2 leftTop,float width,float height) {
	Matrix3x3 result;

	//float width = (vertex.rightTop.x - vertex.leftBottom.x);
	//float height = (vertex.rightTop.y - vertex.leftBottom.y);

	for (int row = 0; row < 3; row++) {
		for (int column = 0; column < 3; column++) {
			result.matrix[row][column] = 0.0f;
		}
	}

	result.matrix[0][0] = width / 2.0f;
	result.matrix[1][1] = -(height / 2.0f);
	result.matrix[2][0] = leftTop.x + (width / 2.0f);
	result.matrix[2][1] = leftTop.y + (height / 2.0f);
	result.matrix[2][2] = 1.0f;

	return result;
}

Vertex4 MakeCameraVertex(Vertex4 vertex,Matrix3x3 targetMatrix,Matrix3x3 worldMatrix,Vertex4 orthographicVertex, Vector2 leftTop, float width, float height) {
	Matrix3x3 viewMatrix = InverseMatrix3x3(worldMatrix);
	Matrix3x3 orthoMatrix = MakeOrthographicMatrix(orthographicVertex);
	Matrix3x3 viewportMatrix = MakeViewportMatrix(leftTop,width,height);

	Matrix3x3 wvpVpMatrix = Matrix3x3Multiply(targetMatrix,viewMatrix);
	wvpVpMatrix = Matrix3x3Multiply(wvpVpMatrix,orthoMatrix);
	wvpVpMatrix = Matrix3x3Multiply(wvpVpMatrix,viewportMatrix);

	Vertex4 result;

	result = VertexTransform(vertex, wvpVpMatrix);

	return result;
}

Vector2 MakeCameraVector2(Vector2 vector2,Matrix3x3 worldMatrix,Vertex4 orthographicVertex, Vector2 leftTop, float width, float height) {
	Matrix3x3 viewMatrix = InverseMatrix3x3(worldMatrix);
	Matrix3x3 orthoMatrix = MakeOrthographicMatrix(orthographicVertex);
	Matrix3x3 viewportMatrix = MakeViewportMatrix(leftTop, width, height);

	 //= Matrix3x3Multiply(targetMatrix, );
	 Matrix3x3 wvpVpMatrix = Matrix3x3Multiply(viewMatrix, orthoMatrix);
	wvpVpMatrix = Matrix3x3Multiply(wvpVpMatrix, viewportMatrix);

	Vector2 result;

	result = Transform(vector2, wvpVpMatrix);

	return result;
}