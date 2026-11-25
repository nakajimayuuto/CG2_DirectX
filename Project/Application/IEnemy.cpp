#include "IEnemy.h"
#include "ProjectileManager.h"

void IEnemy::OnCollision(Collider* other){
	currentHP_ = other->GetDamage();
}

void IEnemy::ApproachUpdate() {
}

void IEnemy::StandByUpdate() {
}

void IEnemy::WithdrawalUpdate() {

}

void IEnemy::ShotUpdate() {
	if (baseShotTimer_ <= 0.0f) {
		ProjectileManager::GetInstance()->CreateDiffusionBullet(
			transform_,
			Vector3(RadianToVector(-transform_.rotate.y + Radian(90.0f)).x,0.0f,RadianToVector(-transform_.rotate.y + Radian(90.0f)).x).Normalize() * baseBulletSpeed_,
			BulletType::kNormal,
			CollisionAttributeName::kCollisionEnemyAttack,
			baseDamage_,
			3.0f,
			baseDiffusionRange_,
			baseDiffusionAmount_
			);
		baseShotTimer_ = baseShotTimerMax_;
	} else {
		baseShotTimer_ -= DeltaTime::GetInstance()->GetGameTime();
	}
}
