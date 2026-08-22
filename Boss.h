#pragma once
#include "Satlib.h"
#include "HPGauge.h"
#include "MirrorHalberd.h"
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

	void SetTargetTransform(Transform* transform) { targetTransform_ = transform; };

	void SetTargetIsAttact(bool isAttack) { isAttack_ = isAttack; };

	static void RegisterGlobalVariables();
	static void ApplyGlobalVariables();

	Vector3 GetWorldPosition() override { return transform_.GetAffineMatrix().GetMatrixToTranslate(); };

	void OnCollision([[maybe_unused]] Collider* other)override;

	bool GetIsChangePhase() { return isChangePhase_; };

	void SetIsChangePhase(bool isChangePhase) { isChangePhase_ = isChangePhase; };

	void SetIsImmune(bool isImmune) { isImmune_ = isImmune; };

	Phase GetPhase() { return phase_; };
private:
	void SlashEffectCreate(Transform* targetTransform, uint32_t num);

	Vector3 GetMoveAnchorPointFindAll();
	Vector3 GetMoveAnchorPointFind(float radius);

	void DistanceCheckUpdate();

	void RootUpdate();

	void EffectUpdate();
private:
	enum class Attacks {
		kWarp,
		kBulletShot,
		kBounsShot,
		kDiffusionShot,
		kMovingShot,
		kWaveShot,
		kSpinningHalberd,
		kPowerSlasher,
		kFangAttack,
		kNearAttack,
		kDown,
		kSuperDown,
		kCountMax,
		kFireBulletShot,

	};

	enum class DistanceName {
		kNear,
		kMiddle,
		kFar,
	};

	struct AttackData {
		Attacks attackName = Attacks::kBulletShot;
		float weight = 0.0f;
		uint32_t continuousCount = 0;
		float magnification = 1.0f;
	};

	std::unique_ptr<MirrorHalberd> halberdLeft_;
	std::unique_ptr<MirrorHalberd> halberdRight_;

	bool isImmune_;

	Model model_;
	Model halberdModel_;

	std::unique_ptr<HPGauge> hpGauge;

	std::vector<Vector3> anchorPoints_; // それぞれのアンカーポイント.

	float destinationAngleY_ = 0.0f;

	Vector3 anchorPointCenter_; // アンカーポイントの中心.

	Transform* targetTransform_ = nullptr; // プレイヤーの位置.

	Transform modelTransform_; // ボスモデルのトランスフォーム.
	Transform halberdTransform_; // 武器のトランスフォーム.

	float deltaTime_;

	DistanceName currentDistance_;

	float currentHP_;
	float maxHP_;

	bool isAttack_;

	bool isChangePhase_;

	Phase phase_;

	std::unique_ptr<Emitter> emitter_;

	std::unique_ptr<Emitter> downEmitter_;

	float debugHpScale_;

	float movingRadius_ = 72.0f;
private:
	void SetAttackData(Attacks attackName, float weight, DistanceName name);

	void AttackSelect(std::vector<AttackData> attackDatas);

	void ClearAttackDatas();

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

	void SuperDownInitialize();
	void SuperDownUpdate();

	void BulletInitialize();
	void BulletUpdate();

	void BounsInitialize();
	void BounsUpdate();

	void DiffusionBulletInitialize();
	void DiffusionBulletUpdate();

	void MovingBulletInitialize();
	void MovingBulletUpdate();

	void WaveInitialize();
	void WaveUpdate();

	void SpinningInitialize();
	void SpinningUpdate();

	void PowerSlasherInitialize();
	void PowerSlasherUpdate();

	void FangAttackInitialize();
	void FangAttackUpdate();
	void FangAttackFangCreate();

	void NearAttackInitialize();
	void NearAttackUpdate();
private:
	static void (Boss::* pInitializeFunc[])();
	static void (Boss::* pUpdateFunc[])();
private:
	std::vector<AttackData> nearAttackDatas_;
	std::vector<AttackData> middleAttackDatas_;
	std::vector<AttackData> farAttackDatas_;
private:
	//Debug.
	bool useDebugUpdateStop = false;



	// Anim
	Transform destinationHalberdTransform_;
	static inline float kDestinationCompletionRate = 0.25f;
	static inline float kBasicPositionY = 2.5f;

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


	static inline Vector3 kBasicColliderSize = { 1.0f,1.6f,1.0f };
	static inline Vector3 kBasicHalberdColliderSize = { 0.6f,2.8f,0.8f };
	static inline float kBasicColliderRadius = 0.5f;
	std::unique_ptr<Collider> attackTempCollider_;
	Transform attackTempTransform_;

	// ダメージ処理.
	float damageCoolTimer_;
	uint32_t damageCountFirst_;
	uint32_t damageCountSecond_;
	uint32_t damageCountThird_;



	// 攻撃全般.
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

	// Warp
	static inline float kWarpEnterTimerMax = 0.5f;
	static inline float kWarpFinishedTimerMax = 0.5f;

	// BulletShot.
	static inline float kBulletStartGapTimerMax = 0.2f;
	static inline float kBulletStayTimerMax = 0.2f;
	static inline float kBulletFinishedGapTimerMax = 0.3f;

	static inline Vector3 kBulletHalberdPos = { -2.0f,0.0f,-3.0f };
	static inline float kBulletAnimRotateY = -45.0f;

	Vector3 bulletShotDirectionTemp_;

	// BounsShot.
	static inline float kBounsStartGapTimerMax = 0.7f;
	static inline float kBounsStayTimerMax = 0.1f;
	static inline float kBounsSpinTimerMax = 0.2f;
	static inline float kBounsFinishedGapTimerMax = 0.5f;

	static inline float kBounsAnimPositionY = 5.0f;
	static inline Vector3 kBounsHalberdStartPos = { -0.5f,3.5f,0.0f };
	static inline Vector3 kBounsHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kBounsHalberdSpinPos = { -3.0f,0.0f,0.0f };
	static inline Vector3 kBounsHalberdSpinRotate = { 0.0f,0.0f,Radian(90.0f) };

	// DiffusionShot.
	static inline float kDiffusionBulletStartGapTimerMax = 0.2f;
	static inline float kDiffusionBulletSpinTimerMax = 0.6f;
	static inline float kDiffusionBulletBackTimerMax = 0.2f;
	static inline float kDiffusionBulletFinishedGapTimerMax = 0.5f;

	static inline Vector3 kDiffusionBulletHalberdStartPos = { 0.0f,0.0f,-2.0f };
	static inline Vector3 kDiffusionBulletHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kDiffusionBulletHalberdSpinRotate = { 0.0f,0.0f,Radian(360.0f) * 3.0f };
	static inline float kDiffusionBulletAnimPositionZ = 1.0f;

	// MovingShot.
	static inline float kMovingBulletStartGapTimerMax = 0.2f;
	static inline float kMovingBulletStayTimerMax = 0.2f;
	static inline float kMovingBulletShotGapTimerMax = 0.2f;
	static inline float kMovingBulletFinishedGapTimerMax = 0.3f;
	static inline float kMovingBulletFinishedTimerMax = 2.0f;

	static inline Vector3 kMovingBulletHalberdPos = { -2.0f,0.0f,-3.0f };
	static inline float kMovingBulletAnchorRadius = 40.0f;

	float movingBulletTimer_ = 0.0f;
	Vector3 movingBulletTargetPos;

	// Wave.
	static inline float kWaveStartGapTimerMax = 0.7f;
	static inline float kWaveStayTimerMax = 0.2f;
	static inline float kWaveAttackTimerMax = 0.1f;
	static inline float kWaveAttackGapTimerMax = 0.7f;
	static inline float kWaveFinishedGapTimerMax = 0.3f;

	static inline float kWaveAnimPositionY = 8.0f;
	static inline Vector3 kWaveHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline Vector3 kWaveHalberdStayPos = { 0.0f,5.0f,-3.0f };
	static inline Vector3 kWaveHalberdAttackRotate = { Radian(-150.0f),0.0f,0.0f};
	static inline Vector3 kWaveHalberdAttackPos = { 0.0f,-2.0f,-4.0f };

	// 目の前でハルバード回転
	// 
	// 上に飛ばす
	// 地面に突き刺す
	// 後隙
	//みたいな感じ


	//Transform halberdModelTransform;

	// SpiningHalberd.
	static inline float kSpinningStartGapTimerMax = 0.2f;
	static inline float kSpinningStayTimerMax = 0.3f;
	static inline float kSpinningSpinStartTimerMax = 0.2f;
	static inline float kSpinningSpinTimerMax = 1.6f;
	static inline float kSpinningSpinFinnishedTimerMax = 0.2f;
	static inline float kSpinningSpinGapTimerMax = 0.7f;
	static inline float kSpinningFinishedGapTimerMax = 0.3f;

	static inline float kSpinningComplateRate = 0.25f;
	static inline float kSpinningSpeed = 15.0f;
	static inline Vector3 kSpinningHalberdStartPos = { -3.0f,0.0f,0.0f };
	static inline Vector3 kSpinningHalberdStartRotate = { 0.0f,0.0f,Radian(90.0f) };
	static inline float kSpinningStartRotateY = Radian(30.0f);
	static inline Vector3 kSpinningHalberdSpinGapPos = { 2.0f,0.0f,-2.0f };
	static inline Vector3 kSpinningHalberdSpinGapRotate = { -Radian(180.0f),0.0f,Radian(90.0f) };
	static inline  float kSpinningSpinGapRotateY = -Radian(30.0f);

	float spinningRotateY = 0.0f;

	// PowerSlasher.
	static inline float kPowerSlasherStartGapTimerMax = 0.2f;
	static inline float kPowerSlasherStayTimerMax = 1.3f;
	static inline float kPowerSlasherStayBlankTimerMax = 0.2f;
	static inline float kPowerSlasherDashTimerMax = 1.0f;
	static inline float kPowerSlasherDashToSlashTimerMax = 0.3f;
	static inline float kPowerSlasherSlashStayTimerMax = 1.2f;
	static inline float kPowerSlasherFinishedGapTimerMax = 0.3f;

	Transform powerSlasherHalberdCenter_;

	static inline float kPowerSlasherSpeed = 40.0f;
	static inline Vector3 kPowerSlasherHalberdStartPos = { 1.0f,0.0f,-1.0f };
	static inline Vector3 kPowerSlasherHalberdStartRotate = { 0.0f,0.0f,-Radian(60.0f) };
	static inline float kPowerSlasherModelStartRotateY = -Radian(30.0f);
	static inline Vector3 kPowerSlasherHalberdAttackPos = { 2.0f,0.0f,0.0f };
	static inline Vector3 kPowerSlasherHalberdAttackRotate = { 0.0f,0.0f,-Radian(90.0f) };
	static inline Vector3 kPowerSlasherHalberdFinishedPos = { 2.0f,0.0f,0.0f };
	static inline Vector3 kPowerSlasherHalberdFinishedRotate = { 0.0f,0.0f,-Radian(90.0f) };
	static inline float kPowerSlasherModelFinishedRotateY = Radian(30.0f);

	static inline float kPowerSlasherNearSlashRadius = 3.0f;
	static inline float kPowerSlasherSlashRadius = (kPowerSlasherSpeed * kPowerSlasherDashToSlashTimerMax);

	//// FangAttack(なんか攻撃作っていくうちにアニメーションのコスト高くなってくな).
	//// ハルバード上投げ.
	//static inline float kFangAttackStartGapTimerMax = 0.3f;
	//// ジャンプする.
	//static inline float kFangAttackJumpTimerMax = 0.3f;
	//// 回収.
	//static inline float kFangAttackStayTimerMax = 0.3f;
	//// 急降下.
	//static inline float kFangAttackFallingTimerMax = 0.3f;
	//// 攻撃の隙.
	//static inline float kFangAttackAttackGapTimerMax = 0.3f;
	//// 元の見た目に戻る.
	//static inline float kFangAttackFinishedGapTimerMax = 0.3f;

	// FangAttack(Waveの奴を上手く改変してWaveを全く新しいアニメーションにする)
	static inline float kFangAttackStartGapTimerMax = 0.3f;
	static inline float kFangAttackSpinTimerMax = 0.3f;
	static inline float kFangAttackAttackTimerMax = 0.3f;
	static inline float kFangAttackAttackGapTimerMax = 0.7f;
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

	// NearAttack.
	static inline float kNearAttackGapTimerMax = 0.7f;
	static inline float kNearAttackFinishedGapTimerMax = 0.2f;

	static inline float kNearFirstStartGapTimerMax = 0.2f;
	static inline float kNearFirstStayTimerMax = 0.5f;
	static inline float kNearFirstAttackTimerMax = 0.25f;
	static inline float kNearFirstAttackGapTimerMax = 0.3f;

	static inline float kNearSecondStartGapTimerMax = 0.2f;
	static inline float kNearSecondStayTimerMax = 0.5f;
	static inline float kNearSecondAttackTimerMax = 0.25f;
	static inline float kNearSecondAttackGapTimerMax = 0.3f;

	static inline float kNearThirdStartGapTimerMax = 0.3f;
	static inline float kNearThirdStayTimerMax = 0.5f;
	static inline float kNearThirdAttackTimerMax = 0.25f;
	static inline float kNearThirdAttackGapTimerMax = 5.0f;

	float randomYFlip = 1.0f;

	Vector3 nearAttackHalPos;
	Vector3 nearAttackHalRotate;
	float nearAttackModelRotateY;

	float nearAttackSecondProbability_;
	float nearAttackThirdProbability_;

	float nearAttackPreTransform_;

	static inline Vector3 kNearFirstHalberdStartPos = { -3.0f,1.5f,-2.0f };
	static inline Vector3 kNearFirstHalberdStartRotate = { 0.0f,0.0f,Radian(60.0f) };
	static inline float kNearFirstModelStartRotateY = Radian(30.0f);
	static inline Vector3 kNearFirstHalberdAttackPos = { 3.0f,-1.5f,-2.0f };
	static inline Vector3 kNearFirstHalberdAttackRotate = { -Radian(180.0f), 0.0f,Radian(60.0f) };
	static inline float kNearFirstModelAttackRotateY = -Radian(30.0f);

	static inline Vector3 kNearSecondHalberdStartPos = { 3.0f,-0.5f,-2.0f };
	static inline Vector3 kNearSecondHalberdStartRotate = { 0.0f,0.0f,Radian(270.0f) };
	static inline Vector3 kNearSecondHalberdAttackPos = { -3.0f,-0.5f,-2.0f };
	static inline Vector3 kNearSecondHalberdAttackRotate = { -Radian(180.0f), 0.0f,Radian(270.0f) };
	static inline float kNearSecondModelAttackRotateY = Radian(30.0f);

	static inline Vector3 kNearThirdHalberdStartPos = { 0.0f,4.5f,-1.0f };
	static inline Vector3 kNearThirdHalberdStartRotate = { 0.0f,0.0f,0.0f };
	static inline float kNearThirdModelStartRotateX = -Radian(360.0f);
	static inline float kNearThirdStartPositionY = 7.0f;
	static inline float kNearThirdModelStartRotateY = 0.0f;
	static inline float kNearThirdModelAttackRotateX = -Radian(150.0f);
	static inline float kNearThirdAttackPositionY = 4.0f;

	static inline float kNearThirdAttackRadiusNum = 30;
	static inline float kNearThirdAttackRadius = 20.0f;
	static inline uint32_t kNearThirdAttackRadiusLoopCount = 4;

	// Down
	static inline float kDonwStartTimer = 0.5f;
	static inline float kDonwStayTimer = 5.0f;
	static inline float kDonwFinsihTimer = 0.5f;

	float donwAnimHalberdVelocityY_;
	Vector3 downAnimHalberdRotate_;
	Vector3 downAnimHalberdPos_;

	static inline Vector3 kDownHalberdPos_ = {-2.0f,-1.0f,0.0f};
	static inline Vector3 kDownHalberdRotate_ = { Radian(30.0f),Radian(0.0f),0.0f };

	static inline float kDownStartPosY = 7.0f;
	static inline float kDownStayRotateX = Radian(270.0f);
	static inline float kDownStayPosY = 0.6f;

	// halberdFarAttack
	float basicHalberdLeftRotateX;
	float basicHalberdRightRotateX;

	float halberdLeftAttackTimer_;
	float halberdRightAttackTimer_;

	static inline float kHalberdFarAttackTimerMax_ = 3.0f;

	static inline float kBasicHalberdFarLeftRotateX = Radian(-90.0f);
	static inline float kBasicHalberdFarRightRotateX = Radian(-90.0f);
	static inline float kBasicHalberdNormalLeftRotateX = 0.0f;
	static inline float kBasicHalberdNormalRightRotateX = 0.0f;
public:
	void StartAnimInitialize();

	void PhaseChangeAnimInitialize();
private:
	void AnimInitialize();

	void NextAnimPhase(float timerMax);

	void StartAnimationUpdate();

	void PhaseChangeAnimationUpdate();
private:
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


	static inline Vector3 kAnimStartHalberdRotate = {0.0f,Radian(-90.0f),0.0f};
	static inline Vector3 kAnimStartHalberdPos = { 0.0f,-10.0f ,-2.0f };

	static inline Vector3 kAnimStartCameraMovePos = { 0.0f,3.0f ,-10.0f };

	static inline Vector3 kAnimStartHalberdSpawnPos = { 0.0f,7.0f ,-2.0f };
	static inline Vector3 kAnimStartHalberdSpawnCameraPos = { 0.0f,11.5f ,-12.0f };

	static inline Vector3 kAnimStartHalberdSetRotate = { Radian(30.0f),Radian(-90.0f),0.0f };
	static inline Vector3 kAnimStartHalberdSetPos = { -2.0f,1.0f ,-2.0f };
	static inline Vector3 kAnimStartHalberdSetCameraPos = {1.0f,4.5f ,-12.0f };

	static inline Vector3 kAnimStartEyeGrownRotate = {0.0f,Radian(315.0f) ,0.0f};
	static inline Vector3 kAnimStartEyeGrownCameraPos = {1.0f,4.5f ,-15.0f };

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
	static inline Vector3 kAnimPhaseChangeFinishHalberdLeftPos = { 3.0f,2.0f ,-2.0f };
	static inline Vector3 kAnimPhaseChangeFinishHalberdRightPos = { -3.0f,2.0f ,-2.0f };
	static inline Vector3 kAnimPhaseChangeFinishHalberdRotate = { Radian(0.0f),Radian(0.0f),Radian(-30.0f) };
};

