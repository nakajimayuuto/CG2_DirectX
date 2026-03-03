#include "Camera.h"

Camera* Camera::GetInstance() {
	static Camera instance;
	return &instance;
}

void Camera::Initialize(float windowWidth, float windowHeight) {
	scale_ = { 1.0f,1.0f,1.0f };
	rotate_ = { 0.0f,0.0,0.0f };
	translate_ = { 0.0f,0.0f,-5.0f };

	windowWidth_ = windowWidth;
	windowHeight_ = windowHeight;

	fovY_ = 0.45f;
	viewportLeftTop_ = { 0.0f,0.0f,0.0f };
	nearClip_ = 0.1f;
	farClip_ = 100.0f;
	minDepth_ = 0.0f;
	maxDepth_ = 1.0f;
}

void Camera::Update() {
	matrix_ = Matrix4x4::MakeAffineMatrix(scale_, rotate_, translate_);
}

Vector3 Camera::GetCameraVector3(Vector3 vector3, Matrix4x4 matrix) {
	Matrix4x4 viewMatrix = matrix_.Inverse();
	Matrix4x4 projectionMatrix = Matrix4x4::MakePerspectiveFovMatrix(fovY_, windowWidth_ / windowHeight_, nearClip_, farClip_);
	Matrix4x4 worldViewProjectionMatrix = matrix * viewMatrix * projectionMatrix;
	Matrix4x4 viewportMatrix = Matrix4x4::MakeViewportMatrix(viewportLeftTop_, windowWidth_, windowHeight_, minDepth_, maxDepth_);

	Vector3 ndcVertex = worldViewProjectionMatrix.MatrixTransform(vector3);
	Vector3 result = viewportMatrix.MatrixTransform(ndcVertex);

	return result;
}

Matrix4x4 Camera::GetWorldViewProjectionMatrix(Matrix4x4 matrix){
	Matrix4x4 viewMatrix = matrix_.Inverse();
	Matrix4x4 projectionMatrix = Matrix4x4::MakePerspectiveFovMatrix(fovY_, windowWidth_ / windowHeight_, nearClip_, farClip_);
	Matrix4x4 worldViewProjectionMatrix = matrix * viewMatrix * projectionMatrix;
	return worldViewProjectionMatrix;
}
