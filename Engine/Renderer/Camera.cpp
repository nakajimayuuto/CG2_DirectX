#include "Camera.h"
#include "../../Managers/InputManager.h"
#include "../Math/Math.h"
#include "../SystemFile/GlobalVariables.h"

Camera* Camera::GetInstance() {
	static Camera instance;
	return &instance;
}

void Camera::Initialize(float windowWidth, float windowHeight) {
	scale_ = { 1.0f,1.0f,1.0f };
	rotate_ = { 0.0f,0.0f,0.0f };
	translate_ = { 0.0f,0.0f,-50.0f };

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

void Camera::Initialize() {
	scale_ = { 1.0f,1.0f,1.0f };
	rotate_ = { 0.0f,0.0f,0.0f };
	translate_ = { 0.0f,0.0f,-50.0f };

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
	bool useMoving = false;

	if (InputManager::GetInstance()->PressKey(DIK_LSHIFT)) {
		useMoving = true;
	}

	if (useMoving) {
		if (InputManager::GetInstance()->PressKey(DIK_RIGHT)) {
			debugTranslate_.x += 0.05f;
		}
		if (InputManager::GetInstance()->PressKey(DIK_LEFT)) {
			debugTranslate_.x -= 0.05f;
		}
		if (InputManager::GetInstance()->PressKey(DIK_UP)) {
			debugTranslate_.z += 0.05f;
		}
		if (InputManager::GetInstance()->PressKey(DIK_DOWN)) {
			debugTranslate_.z -= 0.05f;
		}
		if (InputManager::GetInstance()->PressKey(DIK_SPACE)) {
			debugTranslate_.y += 0.05f;
		}
		if (InputManager::GetInstance()->PressKey(DIK_LCONTROL)) {
			debugTranslate_.y -= 0.05f;
		}
	}else {
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

void Camera::ChangeCameraMode(){
	if (useDebugCamera_) {
		useDebugCamera_ = false;
	} else {
		useDebugCamera_ = true;
	}
}

void Camera::RegisterGlobalVariables() {
	const std::string& groupName = "Camera";

	GlobalVariables::GetInstance()->CreateGroup(groupName);
	// 【デバッグカメラ用】
	useDebugCamera_ = false;

	GlobalVariables::GetInstance()->AddValue(groupName, "FovY", fovY_);
	GlobalVariables::GetInstance()->AddValue(groupName, "ViewportLeftTop", viewportLeftTop_);
	GlobalVariables::GetInstance()->AddValue(groupName, "NearClip", nearClip_);
	GlobalVariables::GetInstance()->AddValue(groupName, "FarClip", farClip_);
	GlobalVariables::GetInstance()->AddValue(groupName, "MinDepth", minDepth_);
	GlobalVariables::GetInstance()->AddValue(groupName, "MaxDepth", maxDepth_);
};

void Camera::ApplyGlobalVariables() {
	const std::string& groupName = "Camera";

	fovY_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "FovY");
	viewportLeftTop_ = GlobalVariables::GetInstance()->GetVector3Value(groupName, "ViewportLeftTop");
	nearClip_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "NearClip");
	farClip_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "FarClip");
	minDepth_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "MinDepth");
	maxDepth_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "MaxDepth");
};
