#pragma once
#include "Satlib.h"
#include "HPGauge.h"
class Boss : public Collider {
public:
	enum class Phase {
		kPhase1,
		kPhase2,
		kPhase3,
		kFinished,
	};

	~Boss();
	void Initialize();

	void Update();

	void Draw();

	Transform* GetTransform() { return &transform_; };

	void SetTargetTransform(Transform* transform) { 
		targetTransform_ = transform; 
	};

	void SetTargetIsAttact(bool isAttack) { isAttack_ = isAttack; };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	Vector3 GetWorldPosition() override { return transform_.GetAffineMatrix().GetMatrixToTranslate(); };

	void OnCollision([[maybe_unused]] Collider* other)override;

	bool GetIsChangePhase() { return isChangePhase_; };

	void SetIsChangePhase(bool isChangePhase) { isChangePhase_ = isChangePhase; };

	void SetIsImmune(bool isImmune) { isImmune_ = isImmune; };

	Phase GetPhase() { return phase_; };

	bool GetIsDeath() { return isDeath_; };

	void FightStart() { isFight_ = true; };

	void EffectUpdate();

	void SetPosition(const Vector3& position) { transform_.translate = position; };
private:
	enum class AnchorPointType {
		kLeft,
		kRight,
		kNear,
		kFar
	};
	void SlashEffectCreate(Transform* targetTransform, uint32_t num);

	Vector3 GetMoveAnchorPointFindAll();
	Vector3 GetMoveAnchorPointFind(float radius);

	void DistanceCheckUpdate();

	void RootUpdate();

	Vector3 GetAnchorPointPos(AnchorPointType type);

	AnchorPointType GetAnchorPointType(AnchorPointType type);
private:
	enum class Attacks {
		kWarp,
		kDown,
		kResetBlock,
		kBulletShot,
		kDash,
		kNearLightning,
		kLightning,
		kMeteor,
		kGroundShake,

		kCountMax,
		kFireBulletShot,

	};

	struct AttackData {
		Attacks attackName = Attacks::kBulletShot;
		float weight = 0.0f;
		uint32_t continuousCount = 0;
		float magnification = 1.0f;
	};

	float bossNameScaleX_;

	bool isDownThreeWayShot_;
	bool isDownAutoHalberd_;
	bool isDownInfinitySlasher_;
	uint32_t downSpecialAttackCount_;

	float downDamageMangification_;

	bool isImmune_;

	Model model_;
	Model halberdModel_;
	Model blockModel_;
	Transform modelBlockTransform_;

	static inline float kBlockRotate_ = Radian(15.0f);

	std::unique_ptr<HPGauge> hpGauge;

	std::vector<Vector3> anchorPoints_; // それぞれのアンカーポイント.

	float destinationAngleY_ = 0.0f;

	Vector3 anchorPointCenter_; // アンカーポイントの中心.

	Transform* targetTransform_ = nullptr; // プレイヤーの位置.

	Transform modelTransform_; // ボスモデルのトランスフォーム.
	Transform halberdTransform_; // 武器のトランスフォーム.

	float deltaTime_;

	float currentHP_;
	float maxHP_;

	bool isAttack_;

	bool isChangePhase_;

	Phase phase_;

	std::unique_ptr<Emitter> emitter_;

	std::unique_ptr<Emitter> downEmitter_;

	std::string bossName_;
	std::string bossNameMirror_;

	float debugHpScale_;

	float movingRadius_ = 72.0f;

	bool isDeath_;
	bool isActive_;
	bool isFight_;

	float startTimer_;
	static inline float kStartTimerMax_ = 1.0f;
private:
	void SetAttackData(Attacks attackName, float weight);

	void AttackSelect(std::vector<AttackData> attackDatas);

	void ClearAttackDatas();

	Vector3 GetBulletDire() {
		Vector3 targetPos = { 0.0f,0.0f,-1.0f };
		if ((targetTransform_->translate - transform_.translate).Length() != 0.0f) {
			targetPos = targetTransform_->translate + Vector3(0.0f, 1.2f, 0.0f);
		}
		return  (targetPos - transform_.translate).Normalize();
	};

	void Phase1Initialize();
	void Phase2Initialize();

	void HalberdStanceUpdate();

	void Phase2HalberdFarAttackUpdate();

	void AttackInitialize();
	void AttackUpdate();
	void AttackFinished();
	void NextAttackPhase(float timerMax);
	void SetCurrentDistanceHalberdTransform();

	void WarpInitialize();
	void WarpUpdate();

	void DownInitialize();
	void DownUpdate();

	void ResetBlockInitialize();
	void ResetBlockUpdate();

	void BulletInitialize();
	void BulletUpdate();

	void DashInitialize();
	void DashUpdate();

	void NearLightningInitialize();
	void NearLightningUpdate();

	void LightningInitialize();
	void LightningUpdate();

	void MeteorInitialize();
	void MeteorUpdate();

	void GroundShakeInitialize();
	void GroundShakeUpdate();

private:
	static void (Boss::* pInitializeFunc[])();
	static void (Boss::* pUpdateFunc[])();
private:
	std::vector<AttackData> attackDatasPhase1_;

private:

	//Debug.
	bool useDebugUpdateStop = false;

	uint32_t attackCount_;
	static inline uint32_t kAttackCountMax_ = 2;

	uint32_t superAttackCount_;
	static inline uint32_t kSuperAttackCountMax_ = 2;

	uint32_t moveAttackCount_;
	static inline uint32_t kMoveAttackCountMax_ = 2;

	// Anim
	Transform destinationHalberdTransform_;
	static inline float kDestinationCompletionRate = 0.25f;
	static inline float kBasicPositionY = 21.0f;

	Vector3 basicHalberdPos = { -1.0f,0.0f,0.0f };
	Vector3 basicHalberdRotate = { -Radian(90.0f) ,0.0f,0.0f };

	static inline Vector3 kBasicHalberdFarPos = { -1.0f,0.0f,0.0f };
	static inline Vector3 kBasicHalberdFarRotate = { -Radian(90.0f) ,0.0f,0.0f };

	static inline float kMiddleRadius = 40.0f;
	static inline Vector3 kBasicHalberdMiddlePos = { 0.0f,0.0f,-1.0f };
	static inline Vector3 kBasicHalberdMiddleRotate = { 0.0f ,0.0f,0.0f };

	static inline float kNearRadius = 20.0f;
	static inline Vector3 kBasicHalberdNearPos = { -1.0f,0.0f,0.0f };
	static inline Vector3 kBasicHalberdNearRotate = { -Radian(90.0f) ,0.0f,0.0f };


	static inline Vector3 kBasicColliderSize = { 0.5f,1.4f,0.35f };
	static inline Vector3 kBasicHalberdColliderSize = { 0.6f,2.8f,0.8f };
	static inline float kBasicColliderRadius = 0.5f;
	std::unique_ptr<Collider> attackTempCollider_;
	Transform attackTempTransform_;


	static inline Vector3 kBasicHalberdLeftPos = { 3.0f,2.0f ,-2.0f };
	static inline Vector3 kBasicHalberdRightPos = { -3.0f,2.0f ,-2.0f };
	Vector3 basicHalberdLeftRotate = { 0.0f,0.0f,0.0f };
	Vector3 basicHalberdRightRotate = { 0.0f,0.0f,0.0f };

	// ダメージ処理.
	float damageCoolTimer_;
	uint32_t damageCountFirst_;
	uint32_t damageCountSecond_;
	uint32_t damageCountThird_;



	// 攻撃全般.
	float damageAmountRecord_;

	bool useSpecialAttack_;


	Attacks currentAttack_ = Attacks::kWarp; // 現在の攻撃.
	std::optional<Attacks> attackRequest_ = std::nullopt; // 次の攻撃リクエスト.
	bool isPlayAttack_ = false; // 攻撃中か.
	float currentAttackTimer_ = 0.0f; // 攻撃のタイマー.
	float kMaxAttackTimer = 0.0f; // 攻撃のタイマー最大値.
	uint32_t currentAttackPhase = 0; // 攻撃のフェーズ.
	float difficultyMagnificationTime = 1.0f; // タイマーの難易度倍率.
	float dopamineSpeed_ = 1.0f; // スーパードパガキモード.
	Transform preTransform_;

	float attackCoolTimer_;
	float attackCoolTimeMax_ = 3.0f;

	static inline float kBulletDamage = 10.0f;
	static inline float kFireBulletDamage = 50.0f;
	static inline float kFireTrajectoryDamage = 15.0f;
	static inline float kSlasherDamage = 25.0f;
	static inline float kSpinDamage = 5.0f;
	static inline float kFangDamage = 15.0f;
	static inline float kWaveDamage = 15.0f;
	static inline float kNearDamage = 10.0f;
	static inline float kNearThirdDamage = 30.0f;

#pragma region Phase1攻撃

	// Warp
	static inline float kWarpEnterTimerMax = 0.5f;
	static inline float kWarpFinishedTimerMax = 0.5f;

	// Down
	static inline float kDonwStartTimer = 0.5f;
	static inline float kDonwStayTimer = 5.0f;
	static inline float kSuperDonwStayTimer = 7.0f;
	static inline float kDonwFinsihTimer = 0.5f;

	float donwAnimHalberdVelocityY_;
	Vector3 downAnimHalberdRotate_;
	Vector3 downAnimHalberdPos_;

	static inline Vector3 kDownHalberdPos_ = { -2.0f,-1.0f,0.0f };
	static inline Vector3 kDownHalberdRotate_ = { Radian(30.0f),Radian(0.0f),0.0f };

	static inline float kDownStartPosY = 28.0f;
	static inline float kDownStayRotateX = Radian(360.0f);

	// BlockReset
	static inline float kBlockResetStartGapTimerMax = 0.15f;
	static inline float kBlockResetFinishedGapTimerMax = 0.15f;

	// Dash.
	static inline float kDashStartGapTimerMax = 1.0f;
	static inline float kDashMoveTimerMax = 1.0f;
	static inline float kDashFinishedGapTimerMax = 0.3f;

	static inline Vector3 kDashHalberdPos = { -2.0f,0.0f,-3.0f };
	static inline float kDashAnimRotateY = -45.0f;

	Vector3 dashBeforePosition_;
	Vector3 dashAfterPosition_;

	// NearLightning.
	static inline float kNearLightningStartGapTimerMax = 0.2f;
	static inline float kNearLightningStayTimerMax = 1.0f;
	static inline float kNearLightningFinishedGapTimerMax = 0.3f;

	// Meteor.
	static inline float kMeteorStartGapTimerMax = 0.2f;
	static inline float kMeteorStayTimerMax = 1.0f;
	static inline float kMeteorFinishedGapTimerMax = 0.3f;

	// NearLightning.
	static inline float kLightningStartGapTimerMax = 0.2f;
	static inline float kLightningStayTimerMax = 1.0f;
	static inline float kLightningFinishedGapTimerMax = 0.3f;

	// GroundShake.
	static inline float kGroundShakeStartGapTimerMax = 2.0f;
	static inline float kGroundShakeStayTimerMax = 0.2f;
	static inline float kGroundShakeFinishedGapTimerMax = 0.3f;

	bool isVisibleGroundShakeWarningBlock_;

	//static inline Vector3 kDashHalberdPos = { -2.0f,0.0f,-3.0f };
	//static inline float kDashAnimRotateY = -45.0f;

	







#pragma endregion Phase1攻撃
	//=================================================================================================================
	//=================================================================================================================
	//=================================================================================================================
	//=================================================================================================================
	//=================================================================================================================
private:
private:
#pragma region Phase2攻撃
	static inline float kPhase2BulletDamage = 10.0f;
	static inline float kPhase2FireBulletDamage = 50.0f;
	static inline float kPhase2FireTrajectoryDamage = 15.0f;
	static inline float kPhase2SlasherDamage = 25.0f;
	static inline float kPhase2SpinDamage = 5.0f;
	static inline float kPhase2FangDamage = 15.0f;
	static inline float kPhase2WaveDamage = 15.0f;
	static inline float kPhase2NearDamage = 10.0f;
	static inline float kPhase2NearThirdDamage = 30.0f;

	Transform bounceLeftHal_;
	Transform bounceRightHal_;

	// 強化 BounsShot.
	static inline float kPhase2BounsStartGapTimerMax = 0.7f;
	static inline float kPhase2BounsStayTimerMax = 0.2f;
	static inline float kPhase2BounsSpinTimerMax = 0.5f;
	static inline float kPhase2BounsFinishedGapTimerMax = 0.5f;

	static inline float kPhase2BounsAnimPositionY = 5.0f;
	static inline Vector3 kPhase2BounsHalberdStartPos = { -0.5f,3.5f,0.0f };
	static inline Vector3 kPhase2BounsHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kPhase2BounsHalberdSpinPos = { -3.0f,0.0f,0.0f };
	static inline Vector3 kPhase2BounsHalberdSpinRotate = { 0.0f,0.0f,Radian(90.0f) };
	static inline Vector3 kPhase2BounsRightHalberdSpinPos = { -3.0f,0.0f,0.0f };
	static inline Vector3 kPhase2BounsRightHalberdSpinRotate = { 0.0f,0.0f,Radian(90.0f) };
	static inline Vector3 kPhase2BounsLeftHalberdSpinPos = { 3.0f,0.0f,0.0f };
	static inline Vector3 kPhase2BounsLeftHalberdSpinRotate = { 0.0f,0.0f,Radian(-90.0f) };

	// 強化 DiffusionShot.
	static inline float kPhase2DiffusionBulletStartGapTimerMax = 0.2f;
	static inline float kPhase2DiffusionBulletSpinStartTimerMax = 0.6f;
	static inline float kPhase2DiffusionBulletSpinTimerMax = 0.2f;
	static inline float kPhase2DiffusionBulletSpinEndTimerMax = 0.6f;
	static inline float kPhase2DiffusionBulletFinishedGapTimerMax = 0.5f;

	static inline Vector3 kPhase2DiffusionBulletHalberdStartPos = { 0.0f,0.0f,-2.0f };
	static inline Vector3 kPhase2DiffusionBulletHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kPhase2DiffusionBulletHalberdSpinRotate = { 0.0f,0.0f,Radian(360.0f) * 3.0f };
	static inline float kPhase2DiffusionBulletAnimPositionZ = 1.0f;

	// 強化 MovingShot.
	static inline float kPhase2MovingBulletStartGapTimerMax = 0.2f;
	static inline float kPhase2MovingBulletStayTimerMax = 0.2f;
	static inline float kPhase2MovingBulletShotGapTimerMax = 0.1f;
	static inline float kPhase2MovingBulletFinishedGapTimerMax = 0.3f;
	static inline float kPhase2MovingBulletFinishedTimerMax = 2.0f;

	static inline Vector3 kPhase2MovingBulletHalberdPos = { -2.0f,0.0f,-3.0f };
	static inline float kPhase2MovingBulletAnchorRadius = 40.0f;


	// Wave.
	Transform waveSpinHalTransform_;
	float waveSpinHalRotateY_;

	static inline float kPhase2WaveStartGapTimerMax = 0.7f;
	static inline float kPhase2WaveStayTimerMax = 0.2f;
	static inline float kPhase2WaveAttackTimerMax = 0.1f;
	static inline float kPhase2WaveAttackGapTimerMax = 0.7f;
	static inline float kPhase2WaveFinishedGapTimerMax = 0.3f;

	static inline float kPhase2WaveAnimPositionY = 8.0f;
	static inline float kPhase2WaveSpinHalPositionY = 5.0f;
	static inline Vector3 kPhase2WaveHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kPhase2WaveHalberdLeftStartPos = { 10.0f,2.0f,-2.0f };
	static inline Vector3 kPhase2WaveHalberdRightStartPos = { -10.0f,2.0f,-2.0f };
	static inline Vector3 kPhase2WaveHalberdStayPos = { 0.0f,5.0f,-3.0f };
	static inline Vector3 kPhase2WaveHalberdAttackRotate = { Radian(-150.0f),0.0f,0.0f };
	static inline Vector3 kPhase2WaveHalberdAttackPos = { 0.0f,-2.0f,-4.0f };
	static inline float kPhase2WaveSpinHalAttackPositionY = -2.0f;

	// SpiningHalberd.
	static inline float kPhase2SpinningStartGapTimerMax = 0.2f;
	static inline float kPhase2SpinningStayTimerMax = 0.3f;
	static inline float kPhase2SpinningSpinStartTimerMax = 0.2f;
	static inline float kPhase2SpinningSpinTimerMax = 1.6f;
	static inline float kPhase2SpinningSpinFinnishedTimerMax = 0.2f;
	static inline float kPhase2SpinningSpinGapTimerMax = 0.7f;
	static inline float kPhase2SpinningFinishedGapTimerMax = 0.3f;

	static inline float kPhase2SpinningBounsTimerMax = 0.25f;

	float spinningBounsTimer_;

	static inline float kPhase2SpinningComplateRate = 0.25f;
	static inline float kPhase2SpinningSpeed = 15.0f;
	static inline Vector3 kPhase2SpinningHalberdStartPos = { -3.0f,0.0f,0.0f };
	static inline Vector3 kPhase2SpinningHalberdStartRotate = { 0.0f,0.0f,Radian(90.0f) };
	static inline float kPhase2SpinningStartRotateY = Radian(30.0f);
	static inline Vector3 kPhase2SpinningHalberdSpinGapPos = { 2.0f,0.0f,-2.0f };
	static inline Vector3 kPhase2SpinningHalberdSpinGapRotate = { -Radian(180.0f),0.0f,Radian(90.0f) };
	static inline  float kPhase2SpinningSpinGapRotateY = -Radian(30.0f);

	// PowerSlasher.
	static inline float kPhase2PowerSlasherStartGapTimerMax = 0.2f;
	static inline float kPhase2PowerSlasherStayTimerMax = 1.3f;
	static inline float kPhase2PowerSlasherStayBlankTimerMax = 0.2f;
	static inline float kPhase2PowerSlasherDashTimerMax = 1.0f;
	static inline float kPhase2PowerSlasherDashToSlashTimerMax = 0.3f;
	static inline float kPhase2PowerSlasherSlashStayTimerMax = 1.2f;
	static inline float kPhase2PowerSlasherFinishedGapTimerMax = 0.3f;

	float powerSlasherRotateY_;
	Vector3 powerSlasherDirection_;

	static inline float kPhase2PowerSlasherSpeed = 40.0f;
	static inline Vector3 kPhase2PowerSlasherHalberdStartPos = { 1.0f,0.0f,-1.0f };
	static inline Vector3 kPhase2PowerSlasherHalberdStartRotate = { 0.0f,0.0f,-Radian(60.0f) };
	static inline float kPhase2PowerSlasherModelStartRotateY = -Radian(30.0f);
	static inline Vector3 kPhase2PowerSlasherHalberdAttackPos = { 2.0f,0.0f,0.0f };
	static inline Vector3 kPhase2PowerSlasherHalberdAttackRotate = { 0.0f,0.0f,-Radian(90.0f) };
	static inline Vector3 kPhase2PowerSlasherHalberdFinishedPos = { 2.0f,0.0f,0.0f };
	static inline Vector3 kPhase2PowerSlasherHalberdFinishedRotate = { 0.0f,0.0f,-Radian(90.0f) };
	static inline float kPhase2PowerSlasherModelFinishedRotateY = Radian(30.0f);

	static inline float kPhase2PowerSlasherNearSlashRadius = 3.0f;
	static inline float kPhase2PowerSlasherSlashRadius = (kPhase2PowerSlasherSpeed * kPhase2PowerSlasherDashToSlashTimerMax);


	// FangAttack(Waveの奴を上手く改変してWaveを全く新しいアニメーションにする)
	static inline float kPhase2FangAttackStartGapTimerMax = 0.3f;
	static inline float kPhase2FangAttackSpinTimerMax = 0.3f;
	static inline float kPhase2FangAttackAttackTimerMax = 0.3f;
	static inline float kPhase2FangAttackAttackGapTimerMax = 0.7f;
	static inline float kPhase2FangAttackFinishedGapTimerMax = 0.3f;

	static inline float kPhase2FangAttackAnimPositionY = 8.0f;
	static inline Vector3 kPhase2FangAttackHalberdPos = { -0.5f,2.5f,0.0f };
	static inline Vector3 kPhase2FangAttackHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kPhase2FangAttackHalberdSpinPos = { 0.0f,2.5f,0.0f };
	static inline float kPhase2FangAttackAttackPositionY = 2.5f;
	static inline Vector3 kPhase2FangAttackHalberdAttackPos = { 0.0f,1.0f,-0.6f };
	static inline float kPhase2FangAttackAttackRotateX = -Radian(150.0f);

	static inline float kPhase2FangAttackRadius = 20.0f;
	static inline uint32_t kPhase2FangAttackLoopCount = 8;
	static inline uint32_t kPhase2FangAttackRadiusNum = 100;
#pragma endregion Phase2攻撃

	// SpecialAttack

	uint32_t specialAttackCount_;

	// PhaseWarp
	static inline float kSpecialAttackWarpEnterTimerMax = 0.5f;
	static inline float kSpecialAttackWarpFinishedTimerMax = 0.5f;
	static inline float kSpecialAttackChargeStartTimerMax = 2.0f;
	static inline float kSpecialAttackFlashTimerMax = 0.3f;


	// ThreeWayWave.
	float preWaveSpinHalRotateY_;

	static inline float kThreeWayWaveStartGapTimerMax = 0.7f;
	static inline float kThreeWayWaveStayTimerMax = 0.2f;
	static inline float kThreeWayWaveAttackTimerMax = 0.1f;
	static inline float kThreeWayWaveAttackGapTimerMax = 0.7f;
	static inline float kThreeWayWaveHalAttackGapTimerMax = 2.0f;
	static inline float kThreeWayWaveFinishedGapTimerMax = 0.3f;

	static inline float kThreeWayWaveAnimPositionY = 8.0f;
	static inline float kThreeWayWaveSpinHalPositionY = 5.0f;
	static inline Vector3 kThreeWayWaveHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kThreeWayWaveHalberdLeftStartPos = { 20.0f,2.0f,-2.0f };
	static inline Vector3 kThreeWayWaveHalberdRightStartPos = { -20.0f,2.0f,-2.0f };
	static inline Vector3 kThreeWayWaveHalberdStayPos = { 0.0f,5.0f,-3.0f };
	static inline Vector3 kThreeWayWaveHalberdAttackRotate = { Radian(-150.0f),0.0f,0.0f };
	static inline Vector3 kThreeWayWaveHalberdAttackPos = { 0.0f,-2.0f,-4.0f };
	static inline float kThreeWayWaveSpinHalAttackPositionY = -2.0f;

	static inline uint32_t kThreeWayWaveCountMax_ = 6;

	// AutoHalberd
	static inline float kAutoHalberdStartGapTimerMax = 0.3f;
	static inline float kAutoHalberdFinishTimerMax = 20.0f;
	static inline float kAutoHalberdFinishGapTimerMax = 0.5f;

	float autoHalberdFinishTimer_;

	bool autoHalberdStop_;

	uint32_t autoHalberdAttackPhase_;
	Attacks autoHalberdAttack_;

	// InfinitySlasher.
	static inline float kInfinitySlasherStartGapTimerMax = 0.5f;
	static inline float kInfinitySlasherHalRotateTimerMax = 5.0f;
	static inline float kInfinitySlasherStayTimerMax = 1.3f;
	static inline float kInfinitySlasherStayBlankTimerMax = 0.2f;
	static inline float kInfinitySlasherDashTimerMax = 1.0f;
	static inline float kInfinitySlasherDashToSlashTimerMax = 0.3f;
	static inline float kInfinitySlasherSlashStayTimerMax = 1.2f;
	static inline float kInfinitySlasherFinishedGapTimerMax = 0.3f;

	uint32_t infinitySlasherSlashCount_;
	static inline uint32_t kInfinitySlasherSlashCountMax = 4;
	uint32_t infinitySlasherSlashPhaseCount_;
	static inline uint32_t kInfinitySlasherSlashPhaseCountMax = 2;
	Transform infinitySlasherHalberdCenter_;
	float preInfinitySlasherHalCenterRotateY_;

	static inline float kInfinitySlasherSpeed = 40.0f;
	static inline Vector3 kInfinitySlasherHalLeftStartPos = { 40.0f,2.0f,-2.0f };
	static inline Vector3 kInfinitySlasherHalRightStartPos = { -40.0f,2.0f,-2.0f };
	static inline float kInfinitySlasherHalCenterStartPosY = 6.0f;
	static inline Vector3 kInfinitySlasherHalberdStartPos = { 1.0f,0.0f,-1.0f };
	static inline Vector3 kInfinitySlasherHalberdStartRotate = { 0.0f,0.0f,-Radian(60.0f) };
	static inline float kInfinitySlasherModelStartRotateY = -Radian(30.0f);
	static inline Vector3 kInfinitySlasherHalberdAttackPos = { 2.0f,0.0f,0.0f };
	static inline Vector3 kInfinitySlasherHalberdAttackRotate = { 0.0f,0.0f,-Radian(90.0f) };
	static inline Vector3 kInfinitySlasherHalberdFinishedPos = { 2.0f,0.0f,0.0f };
	static inline Vector3 kInfinitySlasherHalberdFinishedRotate = { 0.0f,0.0f,-Radian(90.0f) };
	static inline float kInfinitySlasherModelFinishedRotateY = Radian(30.0f);

	static inline float kInfinitySlasherNearSlashRadius = 3.0f;
	static inline float kInfinitySlasherSlashRadius = (kInfinitySlasherSpeed * kInfinitySlasherDashToSlashTimerMax);

	static inline float kInfinitySlasherHalberdJugdeRadius = 5.0f;
public:
	void StartAnimInitialize();

	void PhaseChangeAnimInitialize();

	void DeathAnimInitialize();
private:
	void AnimInitialize();

	void NextAnimPhase(float timerMax);

	void StartAnimationUpdate();

	void PhaseChangeAnimationUpdate();

	void DeathAnimationUpdate();
private:
#pragma region Anim
	// 全体で使う.
	uint32_t animPhase_ = 0;
	float animTimer_;
	float animTimerMax_;
	Transform preCameraTransform_;

	// StartAnim
	static inline float kAnimStartBlankTimerMax = 1.0f; // 始まりの余白.
	static inline float kAnimStartCameraMoveTimerMax = 1.5f; // 敵にカメラが近づく.
	static inline float kAnimStartCameraMoveBlankTimerMax = 0.5f; // カメラが近づいた後の余白.
	static inline float kAnimStartHalberdSpawnTimerMax = 0.7f; // ハルバードが現れる.
	static inline float kAnimStartHalberdSpawnBlankTimerMax = 0.2f; // ハルバードが現れる.
	static inline float kAnimStartHalbardSetPosTimerMax = 0.7f; // ハルバードが敵の手元に移動.
	static inline float kAnimStartHalbardSetPosBlankTimerMax = 0.3f; // 手元に移動した後の余白.
	static inline float kAnimStartEyeGrownTimerMax = 0.5f; // 敵の上田光.
	static inline float kAnimStartEyeGrownBlankTimerMax = 0.5f; // 敵の上田光.
	static inline float kAnimStartJumpTimerMax = 0.5f; // 飛び上がる.
	static inline float kAnimStartJumpBlankTimerMax = 0.5f; // 飛び上がり余白.
	static inline float kAnimStartAttackTimerMax = 0.2f; // 叩きつけ.

	static inline float kAnimStartAttackBlankTimerMax = 2.0f; // 叩きつけの余白.
	static inline float kAnimStartNameShowTimerMax = 0.5f; // 叩きつけで名前出る.
	static inline float kAnimStartFinishTimerMax = 0.5f; // 叩きつけの余白.


	static inline Vector3 kAnimStartHalberdRotate = { 0.0f,Radian(-90.0f),0.0f };
	static inline Vector3 kAnimStartHalberdPos = { 0.0f,-10.0f ,-2.0f };

	static inline Vector3 kAnimStartCameraMovePos = { 0.0f,3.0f ,-10.0f };

	static inline Vector3 kAnimStartHalberdSpawnPos = { 0.0f,7.0f ,-2.0f };
	static inline Vector3 kAnimStartHalberdSpawnCameraPos = { 0.0f,11.5f ,-12.0f };

	static inline Vector3 kAnimStartHalberdSetRotate = { Radian(30.0f),Radian(-90.0f),0.0f };
	static inline Vector3 kAnimStartHalberdSetPos = { -2.0f,1.0f ,-2.0f };
	static inline Vector3 kAnimStartHalberdSetCameraPos = { 1.0f,4.5f ,-12.0f };

	static inline Vector3 kAnimStartEyeGrownRotate = { 0.0f,Radian(315.0f) ,0.0f };
	static inline Vector3 kAnimStartEyeGrownCameraPos = { 1.0f,4.5f ,-15.0f };

	static inline float kAnimStartJumpPosY = 7.0f;
	static inline Vector3 kAnimStartJumpCameraPos = { 0.0f,3.4f ,-23.0f };
	static inline Vector3 kAnimStartJumpCameraRotate = { Radian(-15.0f),0.0f,0.0f };
	static inline Vector3 kAnimStartJumpHalberdPos = { 0.0f,3.0f ,-2.0f };
	static inline Vector3 kAnimStartJumpHalberdRotate = { Radian(0.0f),Radian(0.0f),Radian(0.0f) };

	static inline Vector3 kAnimStartAttackCameraPos = { 0.0f,0.5f ,-23.0f };
	static inline Vector3 kAnimStartAttackCameraRotate = { Radian(-10.0f),0.0f,0.0f };
	static inline Vector3 kAnimStartAttackHalberdPos = { 0.0f,-1.0f ,-4.0f };
	static inline Vector3 kAnimStartAttackHalberdRotate = { Radian(-120.0f),Radian(0.0f),Radian(0.0f) };

	static inline Vector3 kAnimStartNameShowCameraPos = { 0.0f,0.5f ,-25.0f };

	static inline Vector3 kAnimStartFinishHalberdPos = { -2.0f,2.0f ,-2.0f };
	static inline Vector3 kAnimStartFinishHalberdRotate = { Radian(0.0f),Radian(0.0f),Radian(-30.0f) };

	Transform animCameraCenterTransform_;
	Transform animCameraTransform_;

	// PhaseChangeAnim
	static inline float kAnimPhaseChangeCameraRotateTimerMax = 3.0f; // 回転しながらカメラが下りてくる.
	static inline float kAnimPhaseChangeCameraMoveTimerMax = 0.5f; // カメラを近づける.
	static inline float kAnimPhaseChangeHalSpawnTimerMax = 0.7f; // ハルバードが現れる.
	static inline float kAnimPhaseChangeHalRotate90degTimerMax = 0.15f; // ハルバードが回る.
	static inline float kAnimPhaseChangeHalRotate270degTimerMax = 0.15f; // ハルバードが回る.
	static inline float kAnimPhaseChangeHalRotate360degTimerMax = 0.15f; // ハルバードが回る.
	static inline float kAnimPhaseChangeCameraHalRotateTimerMax = 3.0f; // 回転しながらカメラが下りてくる.
	static inline float kAnimPhaseChangeCameraHalRotateBlankTimerMax = 0.5f; // 回転しながらカメラが下りてくる.

	static inline float kAnimPhaseChangeJumpTimerMax = 0.5f; // 飛び上がる.
	static inline float kAnimPhaseChangeJumpBlankTimerMax = 0.5f; // 飛び上がり余白.
	static inline float kAnimPhaseChangeAttackTimerMax = 0.2f; // 叩きつけ.
	static inline float kAnimPhaseChangeAttackBlankTimerMax = 2.0f; // 叩きつけの余白.
	static inline float kAnimPhaseChangeNameShowTimerMax = 0.5f; // 叩きつけで名前出る.
	static inline float kAnimPhaseChangeFinishTimerMax = 0.5f; // 叩きつけの余白.

	static inline Vector3 kAnimPhaseChangeStartHalberdRotate = { 0.0f,Radian(0.0f),0.0f };
	static inline Vector3 kAnimPhaseChangeStartHalberdPos = { 0.0f,-10.0f ,-2.0f };

	static inline float kAnimPhaseChangeCameraMovePosZ_ = -20.0f;
	static inline Vector3 kAnimPhaseChangeHalSpawnHalberdPos = { 0.0f,7.0f ,-2.0f };
	static inline Vector3 kAnimPhaseChangeHalSpawnCameraPos = { 0.0f,11.5f ,-20.0f };
	static inline Vector3 kAnimPhaseChangeHalSpawnHalberdRightPos = { -2.0f,7.0f ,-2.0f };
	static inline Vector3 kAnimPhaseChangeHalSpawnHalberdLeftPos = { 2.0f,7.0f ,-2.0f };


	static inline Vector3 kAnimPhaseChangeCameraHalRotateHalberdPos = { 0.0f,3.0f ,-2.0f };
	static inline Vector3 kAnimPhaseChangeCameraHalRotateHalberdLeftPos = { 2.0f,3.0f ,-2.0f };
	static inline Vector3 kAnimPhaseChangeCameraHalRotateHalberdRightPos = { -2.0f,3.0f ,-2.0f };

	static inline float kAnimPhaseChangeCameraHalRotateHalberdLeftRotateY = Radian(-30.0f);
	static inline float kAnimPhaseChangeCameraHalRotateHalberdRightRotateY = Radian(30.0f);
	static inline Vector3 kAnimPhaseChangeJumpCameraRotate = { Radian(-15.0f),0.0f,0.0f };


	static inline float kAnimPhaseChangeJumpPosY = 7.0f;
	static inline Vector3 kAnimPhaseChangeJumpCameraPos = { 0.0f,3.4f ,-23.0f };
	static inline Vector3 kAnimPhaseChangeJumpHalberdLeftPos = { 2.0f,7.0f ,-2.0f };
	static inline Vector3 kAnimPhaseChangeJumpHalberdRightPos = { -2.0f,7.0f ,-2.0f };

	static inline Vector3 kAnimPhaseChangeAttackCameraPos = { 0.0f,0.5f ,-23.0f };
	static inline Vector3 kAnimPhaseChangeAttackCameraRotate = { Radian(-10.0f),0.0f,0.0f };
	static inline Vector3 kAnimPhaseChangeAttackHalberdPos = { 0.0f,-1.0f ,-4.0f };
	static inline Vector3 kAnimPhaseChangeAttackHalberdRotate = { Radian(-120.0f),Radian(0.0f),Radian(0.0f) };


	static inline Vector3 kAnimPhaseChangeAttackHalberdLeftPos = { 2.5f,-1.0f ,-3.0f };
	static inline Vector3 kAnimPhaseChangeAttackHalberdRightPos = { -2.5f,-1.0f ,-3.0f };

	static inline Vector3 kAnimPhaseChangeNameShowCameraPos = { 0.0f,0.5f ,-25.0f };

	static inline Vector3 kAnimPhaseChangeFinishHalberdPos = { -2.0f,2.0f ,-2.0f };
	static inline Vector3 kAnimPhaseChangeFinishHalberdRotate = { Radian(0.0f),Radian(0.0f),Radian(-30.0f) };


	// DeathAnim
	static inline float kAnimDeathCameraRotateTimerMax = 5.0f; // 回転しながらカメラが下りてくる.
	static inline float kAnimDeathCameraStayTimerMax = 0.5f; // 死亡確定.
	static inline float kAnimDeathScaleTimerMax = 0.1f; // 膨張.
	static inline float kAnimDeathExplodeTimerMax = 1.0f; // 爆発.
	static inline float kAnimDeathExplodeBlankTimerMax = 5.0f; // 爆発.
	static inline float kAnimDeathFinishTimerMax = 2.0f; // フィニッシュ.
	
	float animDeathExplodeTimer_;
	float animDeatExplodeCount_;
	static inline float kAnimDeathExplodeRate = 1.0f;
	static inline float kAnimDeathExplodeCountMax = 3;


	static inline Vector3 kAnimDeathStartHalberdRotate = { 0.0f,Radian(0.0f),0.0f };
	static inline Vector3 kAnimDeathStartHalberdPos = { 0.0f,-10.0f ,-2.0f };

	static inline float kAnimDeathCameraMovePosZ_ = -20.0f;
	static inline Vector3 kAnimDeathHalSpawnHalberdPos = { 0.0f,7.0f ,-2.0f };
	static inline Vector3 kAnimDeathHalSpawnCameraPos = { 0.0f,11.5f ,-20.0f };
	static inline Vector3 kAnimDeathHalSpawnHalberdRightPos = { -2.0f,7.0f ,-2.0f };
	static inline Vector3 kAnimDeathHalSpawnHalberdLeftPos = { 2.0f,7.0f ,-2.0f };


	static inline Vector3 kAnimDeathCameraHalRotateHalberdPos = { 0.0f,3.0f ,-2.0f };
	static inline Vector3 kAnimDeathCameraHalRotateHalberdLeftPos = { 2.0f,3.0f ,-2.0f };
	static inline Vector3 kAnimDeathCameraHalRotateHalberdRightPos = { -2.0f,3.0f ,-2.0f };

	static inline float kAnimDeathCameraHalRotateHalberdLeftRotateY = Radian(-30.0f);
	static inline float kAnimDeathCameraHalRotateHalberdRightRotateY = Radian(30.0f);
	static inline Vector3 kAnimDeathJumpCameraRotate = { Radian(-15.0f),0.0f,0.0f };


	static inline float kAnimDeathJumpPosY = 7.0f;
	static inline Vector3 kAnimDeathJumpCameraPos = { 0.0f,3.4f ,-23.0f };
	static inline Vector3 kAnimDeathJumpHalberdLeftPos = { 2.0f,7.0f ,-2.0f };
	static inline Vector3 kAnimDeathJumpHalberdRightPos = { -2.0f,7.0f ,-2.0f };

	static inline Vector3 kAnimDeathAttackCameraPos = { 0.0f,0.5f ,-23.0f };
	static inline Vector3 kAnimDeathAttackCameraRotate = { Radian(-10.0f),0.0f,0.0f };
	static inline Vector3 kAnimDeathAttackHalberdPos = { 0.0f,-1.0f ,-4.0f };
	static inline Vector3 kAnimDeathAttackHalberdRotate = { Radian(-120.0f),Radian(0.0f),Radian(0.0f) };

	static inline Vector3 kAnimDeathAttackHalberdLeftPos = { 2.5f,-1.0f ,-3.0f };
	static inline Vector3 kAnimDeathAttackHalberdRightPos = { -2.5f,-1.0f ,-3.0f };

	static inline Vector3 kAnimDeathNameShowCameraPos = { 0.0f,0.5f ,-25.0f };

	static inline Vector3 kAnimDeathFinishHalberdPos = { -2.0f,2.0f ,-2.0f };
	static inline Vector3 kAnimDeathFinishHalberdRotate = { Radian(0.0f),Radian(0.0f),Radian(-30.0f) };
#pragma endregion Anim
};

