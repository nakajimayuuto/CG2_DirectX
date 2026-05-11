#include "Transform.h"
#include "Matrix4x4.h"

void Transform::Initialize(){
	scale.x = 1.0f;
	scale.y = 1.0f;
	scale.z = 1.0f;

	rotate.x = 0.0f;
	rotate.y = 0.0f;
	rotate.z = 0.0f;

	translate.x = 0.0f;
	translate.y = 0.0f;
	translate.z = 0.0f;
}

Matrix4x4 Transform::GetAffineMatrix(){
	return Matrix4x4::MakeAffineMatrix(scale,rotate,translate);
}

void Transform2D::Initialize(){
	scale.x = 1.0f;
	scale.y = 1.0f;

	rotate = 0.0f;

	translate.x = 0.0f;
	translate.y = 0.0f;
}
