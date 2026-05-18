#pragma once
#include "Satlib.h"
#include "BaseBullet.h"
#include <list>
#include "RailCameraController.h"

class LockOn;

/// <summary>
/// 自キャラ
/// </summary>
class Player : public Collider{
public:
	~Player();
	void Initialize(const Vector3& position);

	void Update();

	void Draw();

	void OnCollision() override;

	void SetGameScene(IScene* gameScene) { gameScene_ = gameScene; };

	void SetLockOn(LockOn* lockOn) { lockOn_ = lockOn; };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	Vector3 GetWorldPosition() override { return { transform_.GetAffineMatrix().matrix[3][0],transform_.GetAffineMatrix().matrix[3][1],
		transform_.GetAffineMatrix().matrix[3][2] }; };

	Transform GetTransform()const { return transform_; };

	Transform GetTransformReticle2D()const { return transform2DReticle_; };
	Vector2 GetPositionReticle2D()const { return { transform2DReticle_.translate.x,transform2DReticle_.translate.y }; };

	void SetRailCameraController(RailCameraController* cameraController) { cameraController_ = cameraController; transform_.SetParent(&cameraController->GetTransform()); };
private:
	void Reticle2DUpdate();

	void MoveUpdate();

	void RotateUpdate();

	void AttackUpdate();
private:
	static inline float kCharacterSpeed = 0.2f;
	static inline float kRotSpeed = Radian(0.5f);
	static inline float kFirstPointRotSpeed = Radian(1.0f);

	static inline float kMoveLimitX = 20.0f;
	static inline float kMoveLimitY = 11.0f;

	static inline float kRotateLimitX = Radian(10.0f);
	static inline float kRotateLimitY = Radian(15.0f);

	static inline float kFirstPointRotateLimitX = Radian(90.0f);
	static inline float kFirstPointRotateLimitY = Radian(45.0f);

	static inline float kBulletSpeed = 2.0f;

	Transform transform_;

	Renderer::Model model_;

	IScene* gameScene_ = nullptr;
	
	RailCameraController* cameraController_ = nullptr;

	LockOn* lockOn_ = nullptr;

	// 3Dレティクル.
	Renderer::Model model2_;
	Renderer::Sprite sprite_;
	Transform transform3DReticle_;

	Transform transform2DReticle_;

	static inline float kDistancePlayerTo3DReticle_ = 20.0f;
	static inline float kDistancePlayerTo2DReticle_ = 0.1f;
};

