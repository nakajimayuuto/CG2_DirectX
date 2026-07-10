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
	enum class Attacks {
		kWarp,
		kBulletShot,
		kThreeWayShot,
		kFireBulletShot,


	};
	Model model_;

	std::vector<Vector3> anchorPoints_; // それぞれのアンカーポイント.

	float destinationAngleY_ = 0.0f;

	Vector3 anchorPointCenter_; // アンカーポイントの中心.

	Transform* targetTransform_ = nullptr; // プレイヤーの位置.

	Transform transformAxe_; // 武器のトランスフォーム.

	float deltaTime_;
private:
	void AttackInitialize();

	void AttackUpdate();

	void AttackFinished();

	void WarpInitialize();

	void WarpUpdate();

	void BulletInitialize();

	void BulletUpdate();
private:
	static void (Boss::* pInitializeFunc[])();
	static void (Boss::* pUpdateFunc[])();
private:
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


	// Warp
	float kWarpEnterTimerMax = 0.5f;
	float kWarpFinishedTimerMax = 0.5f;


};

