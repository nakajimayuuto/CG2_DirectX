#pragma once
#include "Satlib.h"
class Boss : public Collider {
public:
	~Boss();
	void Initialize();

	void Update();

	void Draw();

	Transform* GetTransform() { return &transform_; };

	void SetTargetTransform(Transform* transform) { targetTransform_ = transform; };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	Vector3 GetWorldPosition() override { return transform_.GetAffineMatrix().GetMatrixToTranslate(); };

	void OnCollision([[maybe_unused]] Collider* other)override;
private:
	Vector3 GetMoveAnchorPointFindAll();
	Vector3 GetMoveAnchorPointFind(float radius);
private:
	enum class Attacks {
		kWarp,
		kBulletShot,
		kBounsShot,
		kDiffusionShot,
		kMovingShot,
		kCountMax,
		kFireBulletShot,

	};
	Model model_;
	Model halberdModel_;

	std::vector<Vector3> anchorPoints_; // それぞれのアンカーポイント.

	float destinationAngleY_ = 0.0f;

	Vector3 anchorPointCenter_; // アンカーポイントの中心.

	Transform* targetTransform_ = nullptr; // プレイヤーの位置.

	Transform modelTransform_; // ボスモデルのトランスフォーム.
	Transform halberdTransform_; // 武器のトランスフォーム.

	float deltaTime_;
private:
	void AttackInitialize();
	void AttackUpdate();
	void AttackFinished();
	void NextAttackPhase(float timerMax);
	void SetCurrentDistanceHalberdTransform();

	void WarpInitialize();
	void WarpUpdate();

	void BulletInitialize();
	void BulletUpdate();

	void BounsInitialize();
	void BounsUpdate();

	void DiffusionBulletInitialize();
	void DiffusionBulletUpdate();

	void MovingBulletInitialize();
	void MovingBulletUpdate();
private:
	static void (Boss::* pInitializeFunc[])();
	static void (Boss::* pUpdateFunc[])();
private:
	// Anim
	Transform destinationHalberdTransform_;
	static inline float kDestinationCompletionRate = 0.25f;
	static inline float kBasicPositionY = 1.4f;

	static inline Vector3 kBasicHalberdFarPos = {-1.0f,0.0f,0.0f};
	static inline Vector3 kBasicHalberdFarRotate = { -Radian(90.0f) ,0.0f,0.0f };

	static inline Vector3 kBasicHalberdMiddlePos = { 0.0f,1.0f,0.0f };


	// 攻撃全般.
	Attacks currentAttack_ = Attacks::kWarp; // 現在の攻撃.
	std::optional<Attacks> attackRequest_ = std::nullopt; // 次の攻撃リクエスト.
	bool isPlayAttack_ = false; // 攻撃中か.
	float currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	float kMaxAttackTimer = 0.0f; // 攻撃のタイマー最大値.
	uint32_t currentAttackPhase = 0; // 攻撃のフェーズ.
	float difficultyMagnificationTime = 1.0f; // タイマーの難易度倍率.
	float difficultyMagnificationDamage = 1.0f; // ダメージの難易度倍率.
	float dopamineSpeed_ = 1.0f; // スーパードパガキモード.
	Transform preTransform_;

	// Warp
	float kWarpEnterTimerMax = 0.5f;
	float kWarpFinishedTimerMax = 0.5f;

	// BulletShot.
	float kBulletStartGapTimerMax = 0.2f;
	float kBulletStayTimerMax = 0.2f;
	float kBulletFinishedGapTimerMax = 0.3f;
	static inline Vector3 kBulletHalberdPos = { -1.0f,0.0f,-3.0f };
	
	static inline float kBulletAnimRotateY = -45.0f;
	

	Vector3 bulletShotDirectionTemp_;

	// BounsShot.
	float kBounsStartGapTimerMax = 0.7f;
	float kBounsStayTimerMax = 0.1f;
	float kBounsSpinTimerMax = 0.2f;
	float kBounsFinishedGapTimerMax = 0.5f;

	static inline float kBounsAnimPositionY = 5.0f;
	static inline Vector3 kBounsHalberdStartPos = { -0.5f,2.5f,0.0f };
	static inline Vector3 kBounsHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kBounsHalberdSpinPos = { -2.0f,0.0f,0.0f };
	static inline Vector3 kBounsHalberdSpinRotate = { 0.0f,0.0f,Radian(90.0f) };

	// DiffusionShot.
	float kDiffusionBulletStartGapTimerMax = 0.2f;
	float kDiffusionBulletSpinTimerMax = 0.6f;
	float kDiffusionBulletBackTimerMax = 0.2f;
	float kDiffusionBulletFinishedGapTimerMax = 0.5f;

	static inline Vector3 kDiffusionBulletHalberdStartPos = { 0.0f,0.0f,-1.5f };
	static inline Vector3 kDiffusionBulletHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kDiffusionBulletHalberdSpinRotate = { 0.0f,0.0f,Radian(360.0f) * 3.0f };
	static inline float kDiffusionBulletAnimPositionZ = 1.0f;

	// MovingShot.
	float kMovingBulletStartGapTimerMax = 0.2f;
	float kMovingBulletStayTimerMax = 0.2f;
	float kMovingBulletShotGapTimerMax = 0.2f;
	float kMovingBulletFinishedGapTimerMax = 0.3f;
	float kMovingBulletFinishedTimerMax = 2.0f;

	static inline Vector3 kMovingBulletHalberdPos = { -1.0f,0.0f,-3.0f };
	static inline float kMovingBulletAnchorRadius = 40.0f;
	float movingBulletTimer_ = 0.0f;
	Vector3 movingBulletTargetPos;

};

