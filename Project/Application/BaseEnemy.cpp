#include "BaseEnemy.h"

Vector3 BaseEnemy::GetWorldPosition(){
	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform_);

	Vector3 worldPos;

	worldPos.x = worldMatrix.matrix[3][0];
	worldPos.y = worldMatrix.matrix[3][1];
	worldPos.z = worldMatrix.matrix[3][2];


	return worldPos;
}