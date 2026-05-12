#define NOMINMAX
#include "Transform.h"
#include "Matrix4x4.h"
#include <algorithm>

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

Matrix4x4 Transform::GetAffineMatrix()const {
	return Matrix4x4::MakeAffineMatrix(scale,rotate,translate);
}

Sphere Transform::GetSphereMin()const {
	Sphere sphere;

	sphere.center = translate;

	sphere.radius = std::min(std::min(scale.x, scale.y), scale.z) / 2.0f;

	return sphere;
}

Sphere Transform::GetSphereMax()const {
	Sphere sphere;

	sphere.center = translate;

	sphere.radius = std::max(std::max(scale.x, scale.y), scale.z) / 2.0f;

	return sphere;
}

void Transform2D::Initialize(){
	scale.x = 1.0f;
	scale.y = 1.0f;

	rotate = 0.0f;

	translate.x = 0.0f;
	translate.y = 0.0f;
}
