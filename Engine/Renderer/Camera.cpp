#define NOMINMAX
#include "Camera.h"
#include "../../Managers/InputManager.h"
#include "../Math/Math.h"
#include "../SystemFile/GlobalVariables.h"
#include "../../Environment.h"
#include "../SystemFile/GameSystem.h"
#include "../Renderer/Renderer.h"
#include "../Math/Collision.h"
#include "../Math/Easing.h"

Camera* Camera::GetInstance() {
	static Camera instance;
	return &instance;
}

void Camera::Initialize(float windowWidth, float windowHeight) {
	windowWidth_ = windowWidth;
	windowHeight_ = windowHeight;

	Initialize();
}

void Camera::Initialize() {
	scale_ = { 1.0f,1.0f,1.0f };
	rotate_ = { 0.0f,0.0f,0.0f };
	translate_ = { 0.0f,0.0f,-50.0f };

	fovY_ = 0.45f;
	viewportLeftTop_ = { 0.0f,0.0f,0.0f };
	nearClip_ = 0.1f;
	farClip_ = 300.0f;
	minDepth_ = 0.0f;
	maxDepth_ = 1.0f;

	// 【デバッグカメラ用】
	useDebugCamera_ = false;

	debugTransformCenter_.Initialize();
	debugTransform_.Initialize();
	debugTransformCenter_.translate = { 0.0f,10.0f,-13.0f };
	debugTransformCenter_.rotate = { Radian(30.0f),0.0f,0.0f };
	//debugTransform_.translate.z = -10.0f;
	debugTransform_.SetParent(&debugTransformCenter_);
	//debugScale_ = { 1.0f,1.0f,1.0f };
	//debugTranslate_ = { 0.0f,0.0f,-10.0f };

	transparentRadiusMax_ = 9.0f;
	transparentRadiusMin_ = 2.0f;
	transparentAlphaMin_ = 0.0f;

	//debugMatRot_ = Matrix4x4::MakeAffineMatrix(debugScale_, rotate_, debugTranslate_);

	spriteTransform.Initialize();
	spriteTransform.translate.x = 640.0f;
	spriteTransform.translate.y = 360.0f;
}

void Camera::CreateResource() {
	cameraResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(CameraForGPU));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
	// 単位行列を書き込んでおく.
	cameraData_->worldPosition = { 0.0f,0.0f,0.0f };
}

void Camera::GetFLog() {
	GameSystem::Log(std::format("Right   : {},{},{}", right.x, right.y, right.z));
	GameSystem::Log(std::format("Up      : {},{},{}", up.x, up.y, up.z));
	GameSystem::Log(std::format("Forward : {},{},{}", forward.x, forward.y, forward.z));
};

bool Camera::IsInCameraFrustum(const Vector3& point, float radius) {
	for (int i = 0; i < 6; i++) {
		float d = static_cast<Vector3>(point).Dot(planes_[i].normal) + planes_[i].distance;

		if (d > radius) {
			return false;
		}
	}

	return true;
}

Vector4 Camera::GetTransparentColor(const Vector3& position, const Vector4& color) {
	if (!Collision::SphereToSphere({ translate_,transparentRadiusMax_ }, { position,0.5f })) {
		return color;
	}

	if (color.w <= transparentAlphaMin_) {
		return color;
	}

	float distance = Vector3::Length(static_cast<Vector3>(translate_) - position);

	Vector4 newColor = color;

	newColor.w = Easing(transparentAlphaMin_, color.w, std::max(distance - transparentRadiusMin_, transparentRadiusMin_), transparentRadiusMax_ - transparentRadiusMin_, EaseType::kConstant);

	return newColor;
}

void Camera::Update() {

	if (useDebugCamera_) {
		DebugUpdate();
		//matrix_ = Matrix4x4::MakeAffineMatrix(debugScale_, debugRotate_, debugTranslate_);
		return;
	}

	cameraData_->worldPosition = translate_;
	matrix_ = Matrix4x4::MakeAffineMatrix(scale_, rotate_, translate_);

	gameCameraMatrix_ = Matrix4x4::MakeAffineMatrix(scale_, rotate_, translate_);
	FrustumUpdate();
}

void Camera::DebugUpdate() {
	Vector3 debugRotate = { 0.0f,0.0f,0.0f };
	bool useMoving = false;
	Vector3 move = { 0.0f,0.0f,0.0f };
	InputManager* input = InputManager::GetInstance();
	if(input->IsGamePadConnect()) {
		move = { input->GetLeftStickDirection().x, 0.0f, input->GetLeftStickDirection().y };

		debugTransformCenter_.rotate.x += -input->GetRightStickDirection().y * Radian(1.0f);
		debugTransformCenter_.rotate.y += input->GetRightStickDirection().x * Radian(1.0f);
	} else {
		if (input->PressKey(DIK_D)) {
			move.x += 1.0f;
		}
		if (input->PressKey(DIK_A)) {
			move.x -= 1.0f;
		}
		if (input->PressKey(DIK_W)) {
			move.z += 1.0f;
		}
		if (input->PressKey(DIK_S)) {
			move.z -= 1.0f;
		}
		if (input->PressKey(DIK_SPACE)) {
			move.y += 1.0f;
		}
		if (input->PressKey(DIK_LSHIFT)) {
			move.y -= 1.0f;
		}

		debugTransformCenter_.rotate.x += input->GetMouse().GetMove().y * Radian(0.1f);
		debugTransformCenter_.rotate.y += input->GetMouse().GetMove().x * Radian(0.1f);
	}

	Matrix4x4 cameraRotateMatrix = Matrix4x4::MakeRotateMatrix(debugTransformCenter_.rotate);

	move = move.Normalize();
	move = cameraRotateMatrix.TransformNomal(move);
	debugTransformCenter_.translate += move * kDebugSpeed;
	matrix_ = debugTransform_.GetAffineMatrix();
	//Matrix4x4 matRotDelta = Matrix4x4::Identity();
	//matRotDelta *= Matrix4x4::MakeRotateYMatrix(debugRotate.x);
	//matRotDelta *= Matrix4x4::MakeRotateXMatrix(debugRotate.y);
	//
	//debugMatRot_ = matRotDelta * debugMatRot_;
	//
	////matrix_ = Matrix4x4::MakeScaleMatrix(debugScale_);
	//matrix_ = debugMatRot_;
	//matrix_ *= Matrix4x4::MakeTranslateMatrix(debugTranslate_);
}

void Camera::FrustumUpdate() {

	Vector3 cameraPos = translate_;
	right = gameCameraMatrix_.GetXAxis().Normalize();
	up = gameCameraMatrix_.GetYAxis().Normalize();
	forward = gameCameraMatrix_.GetZAxis().Normalize();

	float aspect =
		windowWidth_ / windowHeight_;

	float nearHeight =
		2.0f * tanf(fovY_ * 0.5f) * nearClip_;

	float nearWidth =
		nearHeight * aspect;

	float farHeight =
		2.0f * tanf(fovY_ * 0.5f) * farClip_;

	float farWidth =
		farHeight * aspect;

	Vector3 nearCenter =
		cameraPos + forward * nearClip_;

	Vector3 farCenter =
		cameraPos + forward * farClip_;

	float nearHalfW = nearWidth * 0.5f;
	float nearHalfH = nearHeight * 0.5f;

	nearVertex_.leftTop =
		nearCenter
		+ up * nearHalfH
		- right * nearHalfW;

	nearVertex_.rightTop =
		nearCenter
		+ up * nearHalfH
		+ right * nearHalfW;

	nearVertex_.leftBottom =
		nearCenter
		- up * nearHalfH
		- right * nearHalfW;

	nearVertex_.rightBottom =
		nearCenter
		- up * nearHalfH
		+ right * nearHalfW;

	float farHalfW = farWidth * 0.5f;
	float farHalfH = farHeight * 0.5f;

	farVertex_.leftTop =
		farCenter
		+ up * farHalfH
		- right * farHalfW;

	farVertex_.rightTop =
		farCenter
		+ up * farHalfH
		+ right * farHalfW;

	farVertex_.leftBottom =
		farCenter
		- up * farHalfH
		- right * farHalfW;

	farVertex_.rightBottom =
		farCenter
		- up * farHalfH
		+ right * farHalfW;

	// near
	planes_[0].SetPlane(nearVertex_.rightTop, nearVertex_.leftBottom, nearVertex_.leftTop);
	// far
	planes_[1].SetPlane(farVertex_.rightTop, farVertex_.leftTop, farVertex_.leftBottom);
	// left
	planes_[2].SetPlane(farVertex_.leftTop, nearVertex_.leftTop, nearVertex_.leftBottom);
	// right
	planes_[3].SetPlane(farVertex_.rightTop, farVertex_.rightBottom, nearVertex_.rightTop);
	// top
	planes_[4].SetPlane(nearVertex_.rightTop, nearVertex_.leftTop, farVertex_.leftTop);
	// bottom
	planes_[5].SetPlane(farVertex_.rightBottom, farVertex_.leftBottom, nearVertex_.leftBottom);

	ImGui::Begin("Frustm");
	ImGui::Text("Right   : %f %f %f",
		right.x, right.y, right.z);

	ImGui::Text("Up      : %f %f %f",
		up.x, up.y, up.z);

	ImGui::Text("Forward : %f %f %f",
		forward.x, forward.y, forward.z);
	ImGui::End();
}

void Camera::Draw() {
	DrawRange();
}

void Camera::DrawRange() {
	if (!useDebugCamera_) {
		return;
	}

	Renderer::GetInstance()->DrawLine(nearVertex_.leftTop, nearVertex_.rightTop, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine(nearVertex_.rightTop, nearVertex_.rightBottom, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine(nearVertex_.rightBottom, nearVertex_.leftBottom, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine(nearVertex_.leftBottom, nearVertex_.leftTop, { 1.0f,1.0f,1.0f,1.0f });

	Renderer::GetInstance()->DrawLine(farVertex_.leftTop, farVertex_.rightTop, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine(farVertex_.rightTop, farVertex_.rightBottom, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine(farVertex_.rightBottom, farVertex_.leftBottom, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine(farVertex_.leftBottom, farVertex_.leftTop, { 1.0f,1.0f,1.0f,1.0f });

	Renderer::GetInstance()->DrawLine(nearVertex_.leftTop, farVertex_.leftTop, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine(nearVertex_.rightTop, farVertex_.rightTop, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine(nearVertex_.leftBottom, farVertex_.leftBottom, { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawLine(nearVertex_.rightBottom, farVertex_.rightBottom, { 1.0f,1.0f,1.0f,1.0f });
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

Matrix4x4 Camera::GetWorldViewProjectionMatrix(Matrix4x4 matrix) {
	Matrix4x4 viewMatrix = matrix_.Inverse();
	Matrix4x4 projectionMatrix = Matrix4x4::MakePerspectiveFovMatrix(fovY_, windowWidth_ / windowHeight_, nearClip_, farClip_);
	Matrix4x4 worldViewProjectionMatrix = matrix * viewMatrix * projectionMatrix;
	return worldViewProjectionMatrix;
}

Matrix4x4 Camera::GetWorldViewProjectionMatrixSprite(Matrix4x4 matrix) {
	Matrix4x4 viewMatrix;
	Matrix4x4 projectionMatrix;
	Matrix4x4 worldViewProjectionMatrix;
	viewMatrix = spriteTransform.GetTransformValue().GetAffineMatrix();
	projectionMatrix = Matrix4x4::MakeOrthographicMatrix({ viewportLeftTop_,{0.0f,0.0f},{0.0f,0.0f},{windowWidth_,windowHeight_} }, 0.0f, 100.0f);
	worldViewProjectionMatrix = matrix * viewMatrix * projectionMatrix;

	return worldViewProjectionMatrix;
}

Matrix4x4 Camera::GetVPVMatrix(Matrix4x4 matrix) {
	return GetWorldViewProjectionMatrix(matrix) * Matrix4x4::MakeViewportMatrix(viewportLeftTop_, windowWidth_, windowHeight_, minDepth_, maxDepth_);
}

void Camera::ChangeCameraMode() {
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

	//GlobalVariables::GetInstance()->AddValue(groupName, "FovY", fovY_);
	GlobalVariables::GetInstance()->AddValue(groupName, "ViewportLeftTop", viewportLeftTop_);
	GlobalVariables::GetInstance()->AddValue(groupName, "NearClip", nearClip_);
	GlobalVariables::GetInstance()->AddValue(groupName, "FarClip", farClip_);
	GlobalVariables::GetInstance()->AddValue(groupName, "MinDepth", minDepth_);
	GlobalVariables::GetInstance()->AddValue(groupName, "MaxDepth", maxDepth_);
};

void Camera::ApplyGlobalVariables() {
	const std::string& groupName = "Camera";

	//fovY_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "FovY");
	viewportLeftTop_ = GlobalVariables::GetInstance()->GetVector3Value(groupName, "ViewportLeftTop");
	nearClip_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "NearClip");
	farClip_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "FarClip");
	minDepth_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "MinDepth");
	maxDepth_ = GlobalVariables::GetInstance()->GetFloatValue(groupName, "MaxDepth");
};
