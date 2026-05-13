#pragma once
#include "Satlib.h"
#include "BaseBullet.h"
#include <list>

/// <summary>
/// 自キャラ
/// </summary>
class Player : public Collider{
public:
	~Player();
	void Initialize();

	void Update();

	void Draw();

	void OnCollision() override;

	void BulletRemoveCheck();

	const std::list<BaseBullet*>& GetBullet() const { return bullets_; }

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	Vector3 GetWorldPosition() override { return { transform_.GetAffineMatrix().matrix[3][0],transform_.GetAffineMatrix().matrix[3][1],transform_.GetAffineMatrix().matrix[3][2] }; };

	Transform GetTransform()const { return transform_; };
private:
	void MoveUpdate();

	void RotateUpdate();

	void AttackUpdate();
private:
	static inline float kCharacterSpeed = 0.2f;
	static inline float kRotSpeed = Radian(1.0f);

	static inline float kMoveLimitX = 20.0f;
	static inline float kMoveLimitY = 11.0f;

	static inline float kRotateLimitY = Radian(45.0f);

	static inline float kBulletSpeed = 1.0f;
	
	Transform transform_;

	Renderer::Model model_;

	std::list<BaseBullet*> bullets_;
};

