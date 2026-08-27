#pragma once
#include "Satlib.h"
#include "HPGauge.h"
class MirrorHalberd : public Collider{
public:
	enum class HalberdName{
		kLeft,
		kRight,
	};
	void Initialize(HalberdName name);

	void Update();

	void Draw();

	void SetPosition(const Vector3& position) { transform_.translate = position; };

	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; };

	void SetRotateX(float rotateX) { transform_.rotate.x = rotateX; };
	void SetRotateY(float rotateY) { transform_.rotate.y = rotateY; };
	void SetRotateZ(float rotateZ) { transform_.rotate.z = rotateZ; };

	void SetTransform(const Transform& transform) { transform_ = transform; };

	void SetIsActive(bool isActive) { isActive_ = isActive; };

	bool GetIsActive() { return isActive_; };
	bool GetIsColliderActive() { return isColliderActive_; };

	Vector3 GetPosition() { return transform_.translate; };
	Vector3 GetRotate() { return transform_.rotate; };

	Transform GetTransform() { return transform_; };

	void SetParent(Transform* transform) { transform_.SetParent(transform); }; 
	void SetTargetTransform(Transform* transform) { targetTransform_ = transform; };
	void SetBossTransform(Transform* transform) { bossTransform_ = transform; };
	void SetGameTime(float gameTime) { gameTime_ = gameTime; };

	void AutoStart();

	void OnCollision([[maybe_unused]] Collider* other)override;

	void SetTargetIsAttact(bool isAttack) { isAttack_ = isAttack; };

	void AutoAttackStop();

	void SetIsFinished(bool isFinished) { isFinished_ = isFinished; };

	bool GetIsDeath() { return isDeath_; };

	bool GetAutoMove() { return isAutoMove_; };
private:
	Vector3 GetBulletDire() {
		Vector3 targetPos = { 0.0f,0.0f,-1.0f };
		if ((targetTransform_->translate - transform_.translate).Length() != 0.0f) {
			targetPos = targetTransform_->translate + Vector3(0.0f, 1.2f, 0.0f);
		}
		return  (targetPos - transform_.translate).Normalize();
	};

	enum class HalberdAttack{
		kFangShot,
		kWaveShot, 
		kBulletShot,
		kDiffusionShot,
	};
	void AttackInitialize();
	void AttackUpdate();
	void AttackFinished();

	void WaveInitialize();
	void WaveUpdate();

	void FangAttackInitialize();
	void FangAttackUpdate();
	void FangAttackFangCreate();

	void BulletInitialize();
	void BulletUpdate();

	void DiffusionBulletInitialize();
	void DiffusionBulletUpdate();

	void SetAttack();

	void RespawnInitialize();
	void RespawnUpdate();

	void NextAttackPhase(float timerMax) {
		currentAttackTimer_ = 0;
		kMaxAttackTimer = timerMax;
		currentAttackPhase++;
	}
private:
	std::unique_ptr<HPGauge> hpGauge;
	HalberdName name_;

	uint32_t currentPhase_;
	HalberdAttack currentAttack_;

	float gameTime_;

	Model halberdModel_;

	Transform modelTransform_;

	Transform* targetTransform_ = nullptr;

	Transform* bossTransform_ = nullptr;

	float currentHP_;
	float maxHP_;

	float nameScaleX_;
	float scaleTimer_;
	static inline float kScaleTimer = 0.5f;

	bool isDeath_;
	bool isAutoMove_;
	bool isActive_;
	bool isFinished_;

	float currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	float kMaxAttackTimer = 0.0f; // 攻撃のタイマー最大値.
	uint32_t currentAttackPhase = 0; // 攻撃のフェーズ.
	// ダメージ処理.
	float damageCoolTimer_;
	uint32_t damageCountFirst_;
	uint32_t damageCountSecond_;
	uint32_t damageCountThird_;
	bool isAttack_;


	float basicPosY = 2.0f;

	Transform moveTransform_;
	Transform preTransform_;
	Transform preModelTransform_;

	// Start.
	static inline float kStartTimerMax = 0.5f;
	static inline float kAttackGapTimerMax = 0.5f;
	static inline float kAllAttackGapTimerMax = 3.0f;
	static inline float kFinsihGapTimerMax = 0.5f;


	// Wave.
	static inline float kWaveStartGapTimerMax = 0.7f;
	static inline float kWaveStayTimerMax = 0.2f;
	static inline float kWaveAttackTimerMax = 0.1f;
	static inline float kWaveAttackGapTimerMax = kAllAttackGapTimerMax;
	static inline float kWaveFinishedGapTimerMax = 0.3f;

	static inline float kWaveAnimPositionY = 8.0f;
	static inline Vector3 kWaveHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kWaveHalberdStayPos = { 0.0f,5.0f,-3.0f };
	static inline Vector3 kWaveHalberdAttackRotate = { Radian(-150.0f),0.0f,0.0f };
	static inline Vector3 kWaveHalberdAttackPos = { 0.0f,0.0f,-4.0f };

	// FangAttack(Waveの奴を上手く改変してWaveを全く新しいアニメーションにする)
	static inline float kFangAttackStartGapTimerMax = 0.3f;
	static inline float kFangAttackSpinTimerMax = 0.3f;
	static inline float kFangAttackAttackTimerMax = 0.3f;
	static inline float kFangAttackAttackGapTimerMax = kAllAttackGapTimerMax;
	static inline float kFangAttackFinishedGapTimerMax = 0.3f;

	static inline float kFangAttackAnimPositionY = 8.0f;
	static inline Vector3 kFangAttackHalberdPos = { -0.5f,2.5f,0.0f };
	static inline Vector3 kFangAttackHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kFangAttackHalberdSpinPos = { 0.0f,2.5f,0.0f };
	static inline float kFangAttackAttackPositionY = 2.5f;
	static inline Vector3 kFangAttackHalberdAttackPos = { 0.0f,1.0f,-0.6f };
	static inline float kFangAttackAttackRotateX = -Radian(150.0f);

	static inline float kFangAttackRadius = 20.0f;
	static inline uint32_t kFangAttackLoopCount = 8;
	static inline uint32_t kFangAttackRadiusNum = 100;

	Transform halberdModelTransform;
	
	// BulletShot.
	static inline float kBulletStartGapTimerMax = 0.2f;
	static inline float kBulletStayTimerMax = 0.2f;
	static inline float kBulletAttackGapTimerMax = kAllAttackGapTimerMax;
	static inline float kBulletFinishedGapTimerMax = 0.3f;

	static inline Vector3 kBulletHalberdPos = { -2.0f,2.0f,-3.0f };
	static inline float kBulletAnimRotateY = -45.0f;

	static inline Vector3 kBasicHalberdFarPos = { -1.0f,2.0f,0.0f };
	static inline Vector3 kBasicHalberdFarRotate = { -Radian(90.0f) ,0.0f,0.0f };

	Vector3 bulletShotDirectionTemp_;

	// DiffusionShot.
	static inline float kDiffusionBulletStartGapTimerMax = 0.2f;
	static inline float kDiffusionBulletSpinTimerMax = 0.6f;
	static inline float kDiffusionBulletBackTimerMax = 0.2f;
	static inline float kDiffusionBulletAttackGapTimerMax = kAllAttackGapTimerMax;
	static inline float kDiffusionBulletFinishedGapTimerMax = 0.5f;

	static inline Vector3 kDiffusionBulletHalberdStartPos = { 0.0f,0.0f,-2.0f };
	static inline Vector3 kDiffusionBulletHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kDiffusionBulletHalberdSpinRotate = { 0.0f,0.0f,Radian(360.0f) * 3.0f };
	static inline float kDiffusionBulletAnimPositionZ = 1.0f;

	// Respawn.
	static inline float kRespawnStartGapTimerMax = 1.0f;

	static inline float kRespawnHPIncreese = 20.0f;
};

