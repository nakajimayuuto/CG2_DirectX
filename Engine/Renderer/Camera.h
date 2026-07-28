#pragma once
#include "../Math/Matrix4x4.h"
#include "../Math/Transform.h"

#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>

struct CameraForGPU {
	Vector3 worldPosition;	
};

class Camera {
public:

	static Camera* GetInstance();

	void Initialize(float windowWidth, float windowHeight);

	void Initialize();

	void Update();

	void Draw();

	void SetPosition(Vector3 vector3) { translate_ = vector3; }

	void SetRotate(Vector3 rotate) { rotate_ = rotate; };

	void SetTransform(const Transform& transform) { scale_ = transform.scale; rotate_ = transform.rotate; translate_ = transform.translate; };

	void SetAspectScale(const Vector3& aspectScale) { aspectScale_ = aspectScale; };

	Transform GetTransform() { return Transform::GetInitialValue(scale_,rotate_,translate_); };

	Vector3 GetPosition() { return translate_; };

	Vector3 GetRotate() { return rotate_; };

	Vector3 GetCameraVector3(Vector3 vector3,Matrix4x4 matrix);

	Matrix4x4 GetMatrix() {return matrix_;};

	Matrix4x4 GetWorldViewProjectionMatrix(Matrix4x4 matrix);

	Matrix4x4 GetWorldViewProjectionMatrixSprite(Matrix4x4 matrix);	

	Matrix4x4 GetVPVMatrix(Matrix4x4 matrix);

	Matrix4x4 GetViewMatrix() { return  Matrix4x4::MakeAffineMatrix(scale_, rotate_, translate_); };

	Matrix4x4 GetProjectionMatrix() {return Matrix4x4::MakePerspectiveFovMatrix(fovY_, windowWidth_ / windowHeight_, nearClip_, farClip_);}

	void SetWindowSize(float windowWidth, float windowHeight) { windowWidth_ = windowWidth; windowHeight_ = windowHeight; };

	Vector2 GetWindowSize() { return {windowWidth_,windowHeight_}; };

	void ChangeCameraMode();

	bool GetUseDebugCamera() { return useDebugCamera_; };

	void RegisterGlobalVariables();
	void ApplyGlobalVariables();

	Microsoft::WRL::ComPtr<ID3D12Resource> GetCameraForGPUResource() { return cameraResource_; };

	void CreateResource();

	bool IsInCameraFrustum(const Vector3& point, float radius);

	Vector4 GetTransparentColor(const Vector3& position,const Vector4& color);

	void SetFovY(float fovY) { fovY_ = fovY; };
	float GetFovY() { return fovY_; };
private:
	void DebugUpdate();

	void FrustumUpdate();

	void DrawRange();
private:
	static inline float kDebugSpeed = 0.05f;

	Vector3 scale_;
	Vector3 rotate_;
	Vector3 translate_;
	Matrix4x4 matrix_;
	Matrix4x4 worldMatrix;

	Transform2D spriteTransform;

	float windowWidth_;
	float windowHeight_;

	Vertex4 orthographicVertex_;
	Vector3 viewportLeftTop_;
	float viewportWidth_;
	float viewportHeight_;

	float fovY_;
	float nearClip_;
	float farClip_;
	float minDepth_;
	float maxDepth_;

	Transform debugTransform_;
	Transform debugTransformCenter_;

	Matrix4x4 debugMatRot_;

	bool useDebugCamera_;

	Vector3 aspectScale_;

	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_ = nullptr;

	Matrix4x4 gameCameraMatrix_;

	Vertex4 nearVertex_;

	Vertex4 farVertex_;

	Plane planes_[6];

	CameraForGPU* cameraData_ = nullptr;

	float transparentRadiusMax_;
	float transparentRadiusMin_;

	float transparentAlphaMin_;
};