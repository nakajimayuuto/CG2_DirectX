#define NOMINMAX
#include "Transform.h"
#include "Matrix4x4.h"
#include <algorithm>

void Transform::Initialize() {
	scale.x = 1.0f;
	scale.y = 1.0f;
	scale.z = 1.0f;

	rotate.x = 0.0f;
	rotate.y = 0.0f;
	rotate.z = 0.0f;

	translate.x = 0.0f;
	translate.y = 0.0f;
	translate.z = 0.0f;

	parent_ = nullptr;
}


Transform Transform::GetInitialValue(){
	Transform transform;
	transform.Initialize();
	return transform;
}

Transform Transform::GetInitialValue(const Vector3& scale, const Vector3& rotate, const Vector3& translate){
	Transform transform;
	transform.scale = scale;
	transform.rotate = rotate;
	transform.translate = translate;
	return transform;
}

Matrix4x4 Transform::GetAffineMatrix()const {
	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(scale, rotate, translate);
	if (parent_) {
		worldMatrix *= parent_->GetAffineMatrix();
	}
	return worldMatrix;
}

Matrix4x4 Transform::GetScaleMatrix() const{
	Matrix4x4 worldMatrix = Matrix4x4::MakeScaleMatrix(scale);
	if (parent_) {
		worldMatrix *= parent_->GetAffineMatrix();
	}
	return worldMatrix;
}

Matrix4x4 Transform::GetRotateMatrix() const{
	Matrix4x4 worldMatrix = Matrix4x4::MakeRotateXMatrix(rotate.x) * Matrix4x4::MakeRotateYMatrix(rotate.y) *Matrix4x4::MakeRotateZMatrix(rotate.z);
	if (parent_) {
		worldMatrix *= parent_->GetAffineMatrix();
	}
	return worldMatrix;
}

Matrix4x4 Transform::GetTranslateMatrix() const{
	Matrix4x4 worldMatrix = Matrix4x4::MakeTranslateMatrix(translate);
	if (parent_) {
		worldMatrix *= parent_->GetAffineMatrix();
	}
	return worldMatrix;
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

void Transform::TransformSynthesis(const Transform& targetTransform) {
	Matrix4x4 sourceMatrix = this->GetAffineMatrix();
	Matrix4x4 targetMatrix = targetTransform.GetAffineMatrix();

	sourceMatrix *= targetMatrix;

	scale = sourceMatrix.GetMatrixToScale();
	rotate = sourceMatrix.GetMatrixToRotate();
	translate = sourceMatrix.GetMatrixToTranslate();
}

void Transform2D::Initialize() {
	scale.x = 1.0f;
	scale.y = 1.0f;

	rotate = 0.0f;

	translate.x = 0.0f;
	translate.y = 0.0f;
}
