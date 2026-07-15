#pragma once
#include "Satlib.h"
enum class BulletType {
	kNormal,
	kBounce,
	kSpike,
	kFire,
};

class Bullet :Collider{
public:
	void Initialize(const Transform& transform,const Vector3& velocity, BulletType type, CollisionAttributeName colliderName);

	void Update();

	void Draw();

	bool GetIsActive() { return isActive_; };
private:
	static void (Bullet::* pInitializeFunc[])();
	static void (Bullet::* pUpdateFunc[])();

	void NormalInitialize();
	void NormalUpdate();

	void BounsInitialize();
	void BounsUpdate();

private:
	Transform modelTransform_;
	Model model_;
	Vector3 velocity_;
	BulletType type_;

	float lifeTimer_ = 0.0f;
	float lifeTimeMax_ = 3.0f;
	static inline float kBasicLifeTimeMax_ = 3.0f;

	bool isActive_;


	// BounsData
	float kBounsE_ = 0.7f;
	float gravityAcceleration_ = 9.8f;
};

class Explode : Collider {

};

class Spike : Collider {
public:
	/// <summary>
	/// 初期化.
	/// </summary>
	/// <param name="transform">トランスフォーム</param>
	/// <param name="size">サイズ(0で乱数1~3で大きさも決められる)</param>
	/// <param name="colliderName"></param>
	void Initialize(const Transform& transform, uint32_t size, CollisionAttributeName colliderName);

	void Update();

	void Draw();
private:
	Model model_;

	uint32_t spikePhase_;
	float lifeTimer_ = 0.0f;
	float lifeTimeMax_ = 0.0f;
	static inline float kStartTimeMax = 0.3f;
	static inline float kStayTimeMax = 0.4f;
	static inline float kEndTimeMax = 0.3f;

	bool isActive_;

	static inline Vector3 kBasicSpikeSize = {0.2f, 0.5f, 0.2f};
};

class Wave :Collider{
public:
	void Initialize(const Transform& transform, float speed,float height, CollisionAttributeName colliderName);

	void Update();

	void Draw();

	bool GetIsActive() { return isActive_; };
private:
	Model model_;
	float speed_;

	float lifeTimer_ = 0.0f;
	float lifeTimeMax_ = 7.0f;
	static inline float kBasicLifeTimeMax_ = 7.0f;

	bool isActive_;
};

class ProjectileManager {
public:
	static ProjectileManager* GetInstance();

	void Initialize();

	void Update();

	void Draw();

	void CreateBullet(const Transform& transform, const Vector3& velocity, BulletType type, CollisionAttributeName colliderName);

	/// <summary>
	///  Y軸基準で拡散するよ。別に他の軸ができないわけじゃないんですよ。ただ今回のゲームだとつかわないしいいかなって(震え).
	/// </summary>
	/// <param name="transform"></param>
	/// <param name="velocity"></param>
	/// <param name="type"></param>
	/// <param name="diffusionRadian"></param>
	/// <param name="amount"></param>
	void CreateDiffusionBullet(const Transform& transform, const Vector3& velocity, BulletType type, CollisionAttributeName colliderName,float diffusionRadian,uint32_t amount);

	void CreateWave(const Transform& transform, float speed, float height, CollisionAttributeName colliderName);
private:
	std::vector<std::unique_ptr<Bullet>> bullets;
	std::vector<std::unique_ptr<Wave>> waves;
};