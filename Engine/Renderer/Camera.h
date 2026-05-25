#pragma once
#include "../Math/Matrix4x4.h"
#include "../Math/Transform.h"

class Camera{
	Vector3 scale_;
	Vector3 rotate_;
	Vector3 translate_;
	Matrix4x4 matrix_;

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


	Vector3 debugScale_;
	//Vector3 debugRotate_;
	Vector3 debugTranslate_;

	Matrix4x4 debugMatRot_;

	bool useDebugCamera_;
public:

	static Camera* GetInstance();

	void Initialize(float windowWidth, float windowHeight);

	void Initialize();

	void Update();

	void DebugUpdate();
		
	void SetPosition(Vector3 vector3) { translate_ = vector3; }

	void SetRotate(Vector3 rotate) { rotate_ = rotate; };

	void SetTransform(const Transform& transform) { scale_ = transform.scale; rotate_ = transform.rotate; translate_ = transform.translate; };

	Transform GetTransform() { return Transform::GetInitialValue(scale_,rotate_,translate_); };

	Vector3 GetPosition() { return translate_; };

	Vector3 GetCameraVector3(Vector3 vector3,Matrix4x4 matrix);

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
};

