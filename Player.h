#pragma once
#include "Satlib.h"
#include <array>

class MapChipField;

class Enemy;

/// <summary>
/// 自キャラ
/// </summary>
class Player {
public:
	struct CollisionMapInfo {
		bool isCellingCollision = false;
		bool isLanding = false;
		bool isWallCollision = false;
		Vector3 movementAmount;
	};

	void Initialize(const Vector3& position);

	void Update();

	void Draw();

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; };

	Transform GetTransform() { return transform_; };

	Vector3 GetVelocity() { return velocity_; };

	void SetTransform(Transform transform) { transform_ = transform; };

	void SetPosition(Vector3 position) { transform_.translate = position; };

	void ScrollCollision(CollisionMapInfo& info);

	void PlayKDeathMotion() { isKirDeathAnimation_ = true; };

	Vector3	GetWorldPosition()const;

	AABB GetAABB();

	bool GetIsDead() { return isDead_; };

	void OnCollision(const Enemy* enemy);

	bool IsAttack() const;
private:
	enum Corner {
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,

		kNumCornter,
	};

	enum class LRDirection {
		kRight,
		kLeft,
	};

	enum class Behavior {
		kUnknown, // リクエスト無し.
		kRoot, // 通常状態.
		kAttack, // 攻撃中.
	};

	// 1 移動入力.
	void MovingUpdate();

	// 2 移動量を加味して衝突判定を処理.
	void MapCollision(CollisionMapInfo& info);

	void MapCollisionUp(CollisionMapInfo& info);
	void MapCollisionDown(CollisionMapInfo& info);
	void MapCollisionRight(CollisionMapInfo& info);
	void MapCollisionLeft(CollisionMapInfo& info);

	// 3 判定結果を反映して移動.
	void CollisionMoveUpdate(const CollisionMapInfo& info);

	// 4 天井に接触している場合の処理.
	void CellingCollisionUpdate(const CollisionMapInfo& info);

	// 5 壁に接触している場合の処理.
	void IsHitWallUpdate(const CollisionMapInfo& info);

	// 6 接地状態の切り替え.
	void IsGroundUpdate(const CollisionMapInfo& info);

	// 7 旋回制御.
	void TurningControl();

	void CheckFallVoid();

	void KirDeathAnimationUpdate();

	Vector3 CornerPosition(const Vector3& center, Corner corner);

	// 【Behavior】
	// Root.

	void BehaviorRootInitialize();
	void BehaviorRootUpdate();

	// Attack.
	void BehaviorAttackInitialize();
	void BehaviorAttackUpdate();
private:
	/*===========================================================
	Behavior.
	===========================================================*/
	Behavior behavior_ = Behavior::kRoot;
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// Attack.

	enum class AttackPhase {
		kCharge, // 溜め.
		kDash, // 突進.
		kLingeringSound, // 余韻.
	};

	AttackPhase attackPhase_;

	float attackParameter_ = 0.0f;
	static inline const float kAttackParameterCharge = 0.05f;
	static inline const float kAttackParameterDash = 0.2f;
	static inline const float kAttackParameterLingeringSound = 0.05f;

	static inline const float kAttackDashSpeed = 0.6f;

	std::array<Renderer::Model, 2> attackEffectModel_;

	std::array<Transform, 2> attackEffectTransform_;




	// 移動.
	static inline const float kAcceletation = 0.02f;
	static inline const float kAttenuation = 0.1f;
	static inline const float kLimitRunSpeed = 0.25f;

	Transform transform_;
	Vector3 velocity_ = {};

	// ジャンプ.
	static inline const float kGravityAcceleration = 0.01f;
	static inline const float kLimitFallSpeed = 0.4f;
	static inline const float kJumpAcceleration = 0.3f;

	bool onGround_ = true;

	// 旋回制御.
	static inline const float kTimeTurn = 0.3f;

	LRDirection lrDirection_ = LRDirection::kRight;
	float turnFirstRotationY_ = 0.0f;
	float turnTimer_ = 0.0f;

	// マップチップ.
	MapChipField* mapChipField_ = nullptr;

	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	static inline const float kBlank = 0.2f;

	static inline const float kAttenuationLanding = 0.01f;

	static inline const float kAttenuationWall = 0.1f;

	// 死亡判定.
	bool isDead_ = false;

	// デスアニメーション.
	enum class KirAnimationPhase {
		kStop,
		kAnimation,
		kFinish,
	};

	bool isKirDeathAnimation_ = false;

	float kirAnimationTimer_ = 0.0f;

	KirAnimationPhase kirAnimationPhase_ = KirAnimationPhase::kStop;


	// 描画.
	Renderer::Model model_;
};

