#pragma once
#include "Satlib.h"
#include "BaseEntity.h"
class IEnemy : public BaseEntityCharactor {
public:
	void SetTargetTransform(Transform* transform) { targetTransform_ = transform; };

	void SetPosition(const Vector3& position) { transform_.translate = position; };
	void OnCollision([[maybe_unused]] Collider* other) override;
protected:
	void ApproachUpdate();

	void StandByUpdate();

	void WithdrawalUpdate();

	void ShotUpdate();
protected:
	float baseShotTimer_ = 0.0f;
	float baseShotTimerMax_ = 1.0f;
	
	float baseDamage_ = 3.0f;
	float baseDiffusionRange_ = 15.0f;
	uint32_t baseDiffusionAmount_ = 1;
	float baseBulletSpeed_ = 10.0f;



	Transform* targetTransform_;
};