#pragma once
#include "Satlib.h"
enum class BulletType {
	kNormal,
	kBounce,
	kSpike,
	kSlowSpike,
	kBlock,
	kFire,
};

class Bullet :Collider{
public:
	void Initialize(const Transform& transform,const Vector3& velocity, BulletType type, CollisionAttributeName colliderName, float damage, float damageCoolTime);

	void Update();

	void Draw();

	bool GetIsActive() { return isActive_; };

	BulletType GetType() { return type_; };

	Transform GetTransform() { return transform_; };

	void SetDiffusionAmount(uint32_t diffusionAmount) {diffusionAmount_ = diffusionAmount;};
private:
	static void (Bullet::* pInitializeFunc[])();
	static void (Bullet::* pUpdateFunc[])();

	void NormalInitialize();
	void NormalUpdate();

	void BounsInitialize();
	void BounsUpdate();

	void SpikeInitialize();
	void SpikeUpdate();

	void SlowSpikeInitialize();
	void SlowSpikeUpdate();

	void BlockInitialize();
	void BlockUpdate();
private:
	Transform modelTransform_;
	Model model_;
	Vector3 velocity_;
	BulletType type_;
	
	uint32_t diffusionAmount_;

	float lifeTimer_ = 0.0f;
	float lifeTimeMax_ = 3.0f;
	static inline float kBasicLifeTimeMax_ = 5.0f;

	bool isActive_;

	// NormalData
	static inline float kModelRotateSpeed = Radian(90.0f);
	std::unique_ptr<Emitter> emitter_;


	// BounsData
	float kBounsE_ = 0.7f;
	float gravityAcceleration_ = 9.8f;

	// SpikeData
	float spikeCreateTimer_;
	static inline float kSpikeCreateRate = 0.1f;
	static inline Vector3 kRadnomsize_ = { 2.0f,0.0f,2.0f };

	// Block
	static inline float kBlockGravityAccelerationY = 20.0f;
	static inline const float kBlockWidth = 0.05f;
	static inline const float kBlockHeight = 0.05f;

	float spikeRotatePosX_;
	float spikeRotateY_;
	static inline float kSpikeRotateSpeed = Radian(22.5f);
};

class Explode : Collider {
public:
	/// <summary>
	/// 初期化.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="radius">爆発の半径</param>
	/// <param name="colliderName"></param>
	void Initialize(const Transform& transform, float radius, CollisionAttributeName colliderName, float damage, float damageCoolTime);

	void Update();

	bool GetIsActive() { return isActive_; };
private:
	uint32_t phase_;
	float lifeTimer_ = 0.0f;
	float lifeTimeMax_ = 0.0f;
	static inline float kStartTimeMax = 0.1f;
	static inline float kStayTimeMax = 0.4f;

	float maxColliderRadius_;

	bool isActive_;
};

class Spike : Collider {
public:
	/// <summary>
	/// 初期化.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="size">サイズ(0で乱数1~3で大きさも決められる)</param>
	/// <param name="colliderName"></param>
	void Initialize(const Transform& transform, uint32_t size, CollisionAttributeName colliderName, float damage, float damageCoolTime);

	void Update();

	void Draw();

	bool GetIsActive() { return isActive_; };
private:

	Transform modelTransform_;
	uint32_t spikePhase_;
	float lifeTimer_ = 0.0f;
	float lifeTimeMax_ = 0.0f;
	static inline float kStartTimeMax = 0.15f;
	static inline float kStayTimeMax = 0.2f;
	static inline float kEndTimeMax = 0.15f;

	bool isActive_;

	Model model_;

	static inline Vector3 kBasicSpikeSize = {0.4f, 1.0f, 0.4f};
};

class Wave :Collider{
public:
	void Initialize(const Transform& transform, float speed,float height, float time, CollisionAttributeName colliderName, float damage, float damageCoolTime);

	void Update();

	void Draw();

	bool GetIsActive() { return isActive_; };
private:
	Model model_;
	float speed_;

	float heightMax_;
	float height_;
	float lifeTimer_ = 0.0f;
	float lifeTimeMax_ = 7.0f;
	static inline float kBasicLifeTimeMax_ = 7.0f;

	bool isTimeInf_;
	bool isActive_;
};

enum class TutorialObstaclesType {
	kBullet,
	kSpike,
};

class TutorialObstacles :Collider {
public:
	void Initialize(const Transform& transform, CollisionAttributeName colliderName, float damage, float damageCoolTime, TutorialObstaclesType type);

	void Update();

	void Draw();

	bool GetIsActive() { return isActive_; };
private:
	Transform modelTransform_;
	TutorialObstaclesType type_;
	static inline Vector3 kBasicSpikeSize = { 0.4f, 1.0f, 0.4f };
	Model model_;
	bool isActive_;
	static inline float kModelRotateSpeed = Radian(90.0f);
};

class ProjectileManager {
public:
	static ProjectileManager* GetInstance();

	void Initialize();

	void Update();

	void Draw();

	void CreateBullet(const Transform& transform, const Vector3& velocity, BulletType type, CollisionAttributeName colliderName,float damage,float damageCoolTime);
	void CreateBullet(const Transform& transform, const Vector3& velocity, BulletType type, CollisionAttributeName colliderName,float damage,float damageCoolTime,uint32_t diffusionAmount);

	/// <summary>
	///  Y軸基準で拡散するよ。別に他の軸ができないわけじゃないんですよ。ただ今回のゲームだとつかわないしいいかなって(震え).
	/// </summary>
	/// <param name="transform"></param>
	/// <param name="velocity"></param>
	/// <param name="type"></param>
	/// <param name="diffusionRadian"></param>
	/// <param name="amount"></param>
	void CreateDiffusionBullet(const Transform& transform, const Vector3& velocity, BulletType type, CollisionAttributeName colliderName, float damage, float damageCoolTime,float diffusionRadian,uint32_t amount);

	void CreateWave(const Transform& transform, float speed, float height,float time, CollisionAttributeName colliderName, float damage, float damageCoolTime);

	void CreateSpike(const Transform& transform, uint32_t size, CollisionAttributeName colliderName, float damage, float damageCoolTime);

	void CreateExplode(const Transform& transform, float radius, CollisionAttributeName colliderName, float damage, float damageCoolTime);

	void CreateTutorialObstacle(const Vector3& position, TutorialObstaclesType type);
private:
	std::vector<std::unique_ptr<Bullet>> bullets;
	std::vector<std::unique_ptr<Wave>> waves;
	std::vector<std::unique_ptr<Spike>> spikes;
	std::vector<std::unique_ptr<Explode>> explodes;
	std::vector<std::unique_ptr<TutorialObstacles>> tutorials;
	std::vector<std::string> lightNames_;
	const uint32_t kBulletLightMax_ = 30;
};