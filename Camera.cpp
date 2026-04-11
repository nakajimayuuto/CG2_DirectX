#include "Camera.h"
#include "InputManager.h"
#include "Math.h"

Camera* Camera::GetInstance() {
	static Camera instance;
	return &instance;
}

void Camera::Initialize(float windowWidth, float windowHeight) {
	scale_ = { 1.0f,1.0f,1.0f };
	rotate_ = { 0.0f,0.0f,0.0f };
	translate_ = { 0.0f,0.0f,-10.0f };

	windowWidth_ = windowWidth;
	windowHeight_ = windowHeight;

	fovY_ = 0.45f;
	viewportLeftTop_ = { 0.0f,0.0f,0.0f };
	nearClip_ = 0.1f;
	farClip_ = 100.0f;
	minDepth_ = 0.0f;
	maxDepth_ = 1.0f;

	// 【デバッグカメラ用】
	useDebugCamera_ = false;

	debugScale_ = { 1.0f,1.0f,1.0f };
	debugTranslate_ = { 0.0f,0.0f,-10.0f };

	debugMatRot_ = Matrix4x4::MakeAffineMatrix(debugScale_,rotate_,debugTranslate_);
}

void Camera::Update() {
	if (useDebugCamera_) {
		DebugUpdate();

		//matrix_ = Matrix4x4::MakeAffineMatrix(debugScale_, debugRotate_, debugTranslate_);
		return;
	}

	matrix_ = Matrix4x4::MakeAffineMatrix(scale_, rotate_, translate_);
}

void Camera::DebugUpdate(){
	Vector3 debugRotate = { 0.0f,0.0f,0.0f };
	if (InputManager::GetInstance()->PressKey(DIK_RIGHT)) {
		debugRotate.x += Radian(1.0f);
	}
	if (InputManager::GetInstance()->PressKey(DIK_LEFT)) {
		debugRotate.x -= Radian(1.0f);
	}
	if (InputManager::GetInstance()->PressKey(DIK_UP)) {
		debugRotate.y -= Radian(1.0f);
	}
	if (InputManager::GetInstance()->PressKey(DIK_DOWN)) {
		debugRotate.y += Radian(1.0f);
	}

	Matrix4x4 matRotDelta = Matrix4x4::Identity();
	matRotDelta *= Matrix4x4::MakeRotateYMatrix(debugRotate.x);
	matRotDelta *= Matrix4x4::MakeRotateXMatrix(debugRotate.y);

	debugMatRot_ = matRotDelta * debugMatRot_;

	matrix_ = Matrix4x4::MakeScaleMatrix(debugScale_);
	matrix_ *= Matrix4x4::MakeTranslateMatrix(debugTranslate_);
	matrix_ *= debugMatRot_;
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

Matrix4x4 Camera::GetWorldViewProjectionMatrixSprite(Matrix4x4 matrix){
	Matrix4x4 viewMatrix = matrix_.Identity();
	Matrix4x4 projectionMatrix = Matrix4x4::MakeOrthographicMatrix({ {0.0f,0.0f},{0.0f,0.0f},{0.0f,0.0f},{windowWidth_,windowHeight_} }, 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = matrix * viewMatrix * projectionMatrix;
	return worldViewProjectionMatrix;
}
