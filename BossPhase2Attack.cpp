#include "Boss.h"
#include "ProjectileManager.h"
#include "GameCamera.h"



void Boss::Phase2BounsInitialize() {
	kMaxAttackTimer = kPhase2BounsStartGapTimerMax;
	bounceLeftHal_.Initialize();
	bounceLeftHal_.SetParent(&transform_);
	bounceRightHal_.Initialize();
	bounceRightHal_.SetParent(&transform_);
}

void Boss::Phase2BounsUpdate() {
	float randomRadian;
	switch (currentAttackPhase) {
	case 0: // 上昇.
		transform_.translate.y = Easing(kBasicPositionY, kPhase2BounsAnimPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kPhase2BounsHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kPhase2BounsHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		halberdRight_->SetPosition(Easing(kBasicHalberdRightPos, kPhase2BounsRightHalberdSpinPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdRight_->SetRotate(Easing(basicHalberdRightRotate, kPhase2BounsRightHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdLeft_->SetPosition(Easing(kBasicHalberdLeftPos, kPhase2BounsLeftHalberdSpinPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdLeft_->SetRotate(Easing(basicHalberdLeftRotate, kPhase2BounsLeftHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));



		if (currentAttackTimer_ >= kMaxAttackTimer) {
			halberdTransform_.SetParent(&modelTransform_);
			halberdLeft_->SetParent(&bounceLeftHal_);
			halberdRight_->SetParent(&bounceRightHal_);
			NextAttackPhase(kPhase2BounsStayTimerMax);
		}
		break;
	case 1: // 上空で間を開ける.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2BounsSpinTimerMax);
		}
		break;
	case 2: // 回転し初め、遷移時に攻撃を発射.
		modelTransform_.rotate.y = Easing(0.0f, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kPhase2BounsHalberdStartPos, kPhase2BounsHalberdSpinPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kPhase2BounsHalberdStartRotate, kPhase2BounsHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
			NextAttackPhase(kPhase2BounsSpinTimerMax);
			randomRadian = Radian(Random::GetInstance()->RandomFloat(0.0f, 359.0f));
			ProjectileManager::GetInstance()->CreateDiffusionBullet(transform_, Vector3(RadianToVector(randomRadian).x, 0.1f, RadianToVector(randomRadian).y) * 10.0f, BulletType::kBounce, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(45.0f), 8);
		}
		break;
	case 3: // 回転し初め、遷移時に攻撃を発射.
		modelTransform_.rotate.y = Easing(0.0f, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		bounceRightHal_.rotate.y = Easing(0.0f, -Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);


		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
			NextAttackPhase(kPhase2BounsSpinTimerMax);
			randomRadian = Radian(Random::GetInstance()->RandomFloat(0.0f, 359.0f));
			ProjectileManager::GetInstance()->CreateDiffusionBullet(transform_, Vector3(RadianToVector(randomRadian).x, 0.15f, RadianToVector(randomRadian).y) * 10.0f, BulletType::kBounce, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(45.0f), 8);
		}
		break;
	case 4: // 回転し初め、遷移時に攻撃を発射.
		bounceRightHal_.rotate.y = Easing(0.0f, -Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		bounceLeftHal_.rotate.y = Easing(0.0f, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kPhase2BounsHalberdSpinPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(kPhase2BounsHalberdSpinRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
			NextAttackPhase(kPhase2BounsSpinTimerMax);
			randomRadian = Radian(Random::GetInstance()->RandomFloat(0.0f, 359.0f));
			ProjectileManager::GetInstance()->CreateDiffusionBullet(transform_, Vector3(RadianToVector(randomRadian).x, 0.2f, RadianToVector(randomRadian).y) * 10.0f, BulletType::kBounce, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(45.0f), 8);
		}
		break;
	case 5: // 回転を終了する.
		bounceLeftHal_.rotate.y = Easing(0.0f, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2BounsFinishedGapTimerMax);
			transform_.rotate.y = preTransform_.rotate.y;
			halberdTransform_.SetParent(&transform_);
		}
		break;
	case 6: // 降下.
		transform_.translate.y = Easing(kPhase2BounsAnimPositionY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		halberdRight_->SetPosition(Easing(kPhase2BounsRightHalberdSpinPos, kBasicHalberdRightPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdRight_->SetRotate(Easing(kPhase2BounsRightHalberdSpinRotate, basicHalberdRightRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));

		halberdLeft_->SetPosition(Easing(kPhase2BounsLeftHalberdSpinPos, kBasicHalberdLeftPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdLeft_->SetRotate(Easing(kPhase2BounsLeftHalberdSpinRotate, basicHalberdLeftRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::Phase2DiffusionBulletInitialize() {
	kMaxAttackTimer = kPhase2DiffusionBulletStartGapTimerMax;
}

void Boss::Phase2DiffusionBulletUpdate() {
	transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);

	switch (currentAttackPhase) {
	case 0: // ハルバードを前に構える.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kPhase2DiffusionBulletHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kPhase2DiffusionBulletHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
			NextAttackPhase(kPhase2DiffusionBulletSpinStartTimerMax);
		}
		break;
	case 1: // ハルバードを高速回転させる.
		destinationHalberdTransform_.rotate = Easing(kPhase2DiffusionBulletHalberdStartRotate, kPhase2DiffusionBulletHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2DiffusionBulletSpinTimerMax / 2.0f);
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateDiffusionBullet(halberdTransform_, GetBulletDire() * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(10.0f), 3);

		}
		break;
	case 2: // 少し後退
		destinationHalberdTransform_.rotate = Easing(kPhase2DiffusionBulletHalberdStartRotate, kPhase2DiffusionBulletHalberdSpinRotate / 2.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant);
		modelTransform_.translate.z = Easing(0.0f, kPhase2DiffusionBulletAnimPositionZ, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2DiffusionBulletSpinTimerMax / 2.0f);
		}
		break;
	case 3: // 元の位置に戻る.
		destinationHalberdTransform_.rotate = Easing(kPhase2DiffusionBulletHalberdSpinRotate / 2.0f, kPhase2DiffusionBulletHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant);
		modelTransform_.translate.z = Easing(kPhase2DiffusionBulletAnimPositionZ, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateDiffusionBullet(halberdTransform_, GetBulletDire() * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(10.0f), 4);
			NextAttackPhase(kPhase2DiffusionBulletSpinTimerMax / 2.0f);
		}
		break;
	case 4: // 少し後退
		destinationHalberdTransform_.rotate = Easing(kPhase2DiffusionBulletHalberdStartRotate, kPhase2DiffusionBulletHalberdSpinRotate / 2.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant);
		modelTransform_.translate.z = Easing(0.0f, kPhase2DiffusionBulletAnimPositionZ, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2DiffusionBulletSpinTimerMax / 2.0f);
		}
		break;
	case 5: // 元の位置に戻る.
		destinationHalberdTransform_.rotate = Easing(kPhase2DiffusionBulletHalberdSpinRotate / 2.0f, kPhase2DiffusionBulletHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant);
		modelTransform_.translate.z = Easing(kPhase2DiffusionBulletAnimPositionZ, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateDiffusionBullet(halberdTransform_, GetBulletDire() * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(10.0f), 5);
			NextAttackPhase(kPhase2DiffusionBulletSpinTimerMax / 2.0f);
		}
		break;
	case 6: // 少し後退
		destinationHalberdTransform_.rotate = Easing(kPhase2DiffusionBulletHalberdStartRotate, kPhase2DiffusionBulletHalberdSpinRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.translate.z = Easing(0.0f, kPhase2DiffusionBulletAnimPositionZ, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2DiffusionBulletSpinEndTimerMax);
		}
		break;
	case 7: // 元の位置に戻る.
		modelTransform_.translate.z = Easing(kPhase2DiffusionBulletAnimPositionZ, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2DiffusionBulletSpinEndTimerMax);
		}
		break;
	case 8: // 見た目を戻す.
		destinationHalberdTransform_.translate = Easing(kPhase2DiffusionBulletHalberdStartPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kPhase2DiffusionBulletHalberdStartRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}
}

void Boss::Phase2MovingBulletInitialize() {
	kMaxAttackTimer = kPhase2MovingBulletStartGapTimerMax;
	movingBulletTimer_ = 0.0f;
	movingBulletTargetPos = GetMoveAnchorPointFind(kPhase2MovingBulletAnchorRadius);
}

void Boss::Phase2MovingBulletUpdate() {
	movingBulletTimer_ += deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
	transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
	Vector3 direction = { 0.0f,0.0f,-1.0f };
	transform_.translate = Easing(preTransform_.translate, movingBulletTargetPos, movingBulletTimer_, kPhase2MovingBulletFinishedTimerMax, EaseType::kEaseOut);

	switch (currentAttackPhase) {
	case 0:// ハルバードを構える.
		modelTransform_.rotate.y = Easing(0.0f, Radian(kBulletAnimRotateY), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kBulletHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kBasicHalberdFarRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2MovingBulletStayTimerMax);
		}
		break;
	case 1:// 攻撃を発射させる間.
	case 2:
	case 3:
	case 4:
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2MovingBulletShotGapTimerMax);
			if ((targetTransform_->translate - destinationHalberdTransform_.GetWorldPosition()).Length() != 0.0f) {
				direction = (targetTransform_->translate - destinationHalberdTransform_.GetWorldPosition()).Normalize();
			}
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, direction * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 0.5f);
		}
		break;
	case 5: // 攻撃を発射させる間(3回目).
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2MovingBulletFinishedGapTimerMax);
			if ((targetTransform_->translate - destinationHalberdTransform_.GetWorldPosition()).Length() != 0.0f) {
				direction = (targetTransform_->translate - destinationHalberdTransform_.GetWorldPosition()).Normalize();
			}
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, direction * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 0.5f);
		}
		break;
	case 6: // 見た目を戻す.
		modelTransform_.rotate.y = Easing(Radian(kBulletAnimRotateY), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kBulletHalberdPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(kBasicHalberdFarRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2MovingBulletFinishedGapTimerMax);
		}
		break;
	case 7:
		break;
	}

	if (movingBulletTimer_ >= kPhase2MovingBulletFinishedTimerMax) {
		// 一定の時間経過後攻撃終了.
		AttackFinished();
	}
}

void Boss::Phase2WaveInitialize() {
	waveSpinHalTransform_.Initialize();
	waveSpinHalTransform_.SetParent(&transform_);
	kMaxAttackTimer = kPhase2WaveStartGapTimerMax;
	movingBulletTimer_ = 0.0f;
	movingBulletTargetPos = GetMoveAnchorPointFind(kPhase2MovingBulletAnchorRadius);
	halberdTransform_.SetParent(&modelTransform_);
	halberdLeft_->SetParent(&waveSpinHalTransform_);
	halberdRight_->SetParent(&waveSpinHalTransform_);

	waveSpinHalRotateY_ = Random::GetInstance()->RandomFloat(Radian(-360.0f), Radian(360.0f));

	if (std::abs(waveSpinHalRotateY_) < Radian(180.0f)) {
		if (waveSpinHalRotateY_ >= 0.0f) {
			waveSpinHalRotateY_ += Radian(360.0f);
		} else {
			waveSpinHalRotateY_ += Radian(-360.0f);
		}
	}
}

void Boss::Phase2WaveUpdate() {

	switch (currentAttackPhase) {
	case 0: // ハルバードを上昇.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kPhase2WaveHalberdStayPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kPhase2WaveHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(0.0f, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		halberdLeft_->SetPosition(Easing(kBasicHalberdLeftPos, kPhase2WaveHalberdLeftStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdLeft_->SetRotate(Easing(basicHalberdLeftRotate, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdRight_->SetPosition(Easing(kBasicHalberdRightPos, kPhase2WaveHalberdRightStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdRight_->SetRotate(Easing(basicHalberdRightRotate, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		waveSpinHalTransform_.rotate.y = Easing(0.0f, waveSpinHalRotateY_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		waveSpinHalTransform_.translate.y = Easing(0.0f, kPhase2WaveSpinHalPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2WaveStayTimerMax);
		}
		break;
	case 1: // ハルバードを上昇.
		destinationHalberdTransform_.rotate = Easing(kPhase2WaveHalberdStartRotate, kPhase2WaveHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		//waveSpinHalTransform_.translate.y = Easing(kPhase2WaveSpinHalPositionY / 2.0f, kPhase2WaveSpinHalPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2WaveAttackTimerMax);
		}
		break;
	case 2: // 攻撃態勢に入りながら急降下.
		destinationHalberdTransform_.translate = Easing(kPhase2WaveHalberdStayPos, kPhase2WaveHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		//waveSpinHalTransform_.rotate.x = Easing(Radian(0.0f), kPhase2WaveHalberdAttackRotate.x, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		//waveSpinHalTransform_.translate.y = Easing(kPhase2WaveSpinHalPositionY, kPhase2WaveSpinHalAttackPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2WaveAttackGapTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateWave(destinationHalberdTransform_, 25.0f, 1.0f, -1.0f, kCollisionEnemyAttack, 15.0f, 3.0f);
		}
		break;
	case 3: // 攻撃後の後隙.
		halberdLeft_->SetRotateX(Easing(Radian(0.0f), kPhase2WaveHalberdAttackRotate.x, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		halberdRight_->SetRotateX(Easing(Radian(0.0f), kPhase2WaveHalberdAttackRotate.x, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2WaveAttackTimerMax);
		}
		break;
	case 4: // 攻撃後の後隙.
		waveSpinHalTransform_.translate.y = Easing(kPhase2WaveSpinHalPositionY, kPhase2WaveSpinHalAttackPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2WaveAttackGapTimerMax);
			if (std::abs(waveSpinHalRotateY_) > Radian(360.0f)) {
				if (waveSpinHalRotateY_ >= 0.0f) {
					waveSpinHalRotateY_ -= Radian(360.0f);
				} else {
					waveSpinHalRotateY_ -= Radian(-360.0f);
				}
			}
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateWave(halberdLeft_->GetTransform(), 15.0f, 1.0f, 3.0f, kCollisionEnemyAttack, 10.0f, 3.0f);
			ProjectileManager::GetInstance()->CreateWave(halberdRight_->GetTransform(), 15.0f, 1.0f, 3.0f, kCollisionEnemyAttack, 10.0f, 3.0f);

		}
		break;
	case 5: // 攻撃後の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2WaveFinishedGapTimerMax);
		}
		break;
	case 6: // 見た目を戻す.
		destinationHalberdTransform_.translate = Easing(kPhase2WaveHalberdAttackPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kPhase2WaveHalberdAttackPos, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		halberdLeft_->SetRotateX(Easing(kPhase2WaveHalberdAttackRotate.x, Radian(0.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		halberdRight_->SetRotateX(Easing(kPhase2WaveHalberdAttackRotate.x, Radian(0.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));

		waveSpinHalTransform_.rotate.y = Easing(waveSpinHalRotateY_, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		waveSpinHalTransform_.translate.y = Easing(kPhase2WaveSpinHalPositionY, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		halberdLeft_->SetPosition(Easing(kPhase2WaveHalberdLeftStartPos, kBasicHalberdLeftPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdLeft_->SetRotate(Easing({ 0.0f,0.0f,0.0f }, basicHalberdLeftRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdRight_->SetPosition(Easing(kPhase2WaveHalberdRightStartPos, kBasicHalberdRightPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdRight_->SetRotate(Easing({ 0.0f,0.0f,0.0f }, basicHalberdRightRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			modelTransform_.rotate.y = 0.0f;
			halberdTransform_.SetParent(&transform_);
		}
		break;
	}
}

void Boss::Phase2SpinningInitialize() {
	spinningBounsTimer_ = 0.0f;
	kMaxAttackTimer = kPhase2SpinningStartGapTimerMax;
	attackTempTransform_.translate = { 1.0f,0.0f,0.0f };
	attackTempTransform_.SetParent(&transform_);
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);
	attackTempCollider_->SetColliderType(ColliderType::kBox);
	attackTempCollider_->SetDimensionType(ColliderDimensionType::k3D);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack));
	attackTempCollider_->SetCollisionMask(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer));
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	attackTempCollider_->SetDamage(5.0f);
	attackTempCollider_->SetDamageCoolTime(0.1f);
	attackTempCollider_->SetActive(true);
	spinningRotateY = 0.0f;
	isColliderActive_ = false;
}

void Boss::Phase2SpinningUpdate() {
	Transform effectTransform;
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);

	Vector3 move = { 0.0f,0.0f,kPhase2SpinningSpeed };

	spinningRotateY = LerpShortAngle(spinningRotateY, std::atan2(targetTransform_->translate.x - transform_.translate.x, targetTransform_->translate.z - transform_.translate.z), kSpinningComplateRate);

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix({ 0.0f,spinningRotateY,0.0f });

	move = rotateMatrix.TransformNomal(move);

	switch (currentAttackPhase) {
	case 0: // ハルバードを構える.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kPhase2SpinningHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kPhase2SpinningHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = Easing(preTransform_.rotate.y, preTransform_.rotate.y + kPhase2SpinningStartRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2SpinningStayTimerMax);
		}
		break;
	case 1: // 前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2SpinningSpinStartTimerMax);
		}
		break;
	case 2: // 回転し初め.
		spinningBounsTimer_ += deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		if (spinningBounsTimer_ >= kPhase2SpinningBounsTimerMax * difficultyMagnificationTime * dopamineSpeed_) {
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
			spinningBounsTimer_ -= kPhase2SpinningBounsTimerMax;
		}

		effectTransform = attackTempTransform_;
		effectTransform.scale = kBasicHalberdColliderSize;
		effectTransform.translate.y = 0.0f;
		SlashEffectCreate(&effectTransform, 3);
		transform_.rotate.y = Easing(preTransform_.rotate.y + kPhase2SpinningStartRotateY, preTransform_.rotate.y + kPhase2SpinningStartRotateY - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2SpinningSpinTimerMax);
			transform_.rotate.y = preTransform_.rotate.y + kPhase2SpinningStartRotateY;
		}
		break;
	case 3: // 回転の最中.
		spinningBounsTimer_ += deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		effectTransform = attackTempTransform_;
		effectTransform.scale = kBasicHalberdColliderSize;
		effectTransform.translate.y = 0.0f;
		SlashEffectCreate(&effectTransform, 3);
		if (spinningBounsTimer_ >= kPhase2SpinningBounsTimerMax * difficultyMagnificationTime * dopamineSpeed_) {
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateBullet(transform_, { 0.0f,Random::GetInstance()->RandomFloat(7.0f,10.0f),0.0f }, BulletType::kBounce, kCollisionEnemyAttack, 10.0f, 3.0f);
			spinningBounsTimer_ -= kPhase2SpinningBounsTimerMax;
		}

		transform_.rotate.y = Easing(preTransform_.rotate.y + kPhase2SpinningStartRotateY, preTransform_.rotate.y + kPhase2SpinningSpinGapRotateY - (Radian(360.0f) * 10.0f * difficultyMagnificationTime * dopamineSpeed_), currentAttackTimer_, kMaxAttackTimer * difficultyMagnificationTime * dopamineSpeed_, EaseType::kConstant);
		if (currentAttackTimer_ >= kMaxAttackTimer * difficultyMagnificationTime * dopamineSpeed_) {
			NextAttackPhase(kPhase2SpinningSpinFinnishedTimerMax);
			transform_.rotate.y = preTransform_.rotate.y + kPhase2SpinningSpinGapRotateY;
		}
		break;
	case 4: // 回転し終わり.
		effectTransform = attackTempTransform_;
		effectTransform.scale = kBasicHalberdColliderSize;
		effectTransform.translate.y = 0.0f;
		SlashEffectCreate(&effectTransform, 3);
		transform_.rotate.y = Easing(preTransform_.rotate.y + kPhase2SpinningSpinGapRotateY, preTransform_.rotate.y + kPhase2SpinningSpinGapRotateY - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.translate = Easing(kPhase2SpinningHalberdStartPos, kPhase2SpinningHalberdSpinGapPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kPhase2SpinningHalberdStartRotate, kPhase2SpinningHalberdSpinGapRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2SpinningStayTimerMax);
			transform_.rotate.y = preTransform_.rotate.y + kPhase2SpinningSpinGapRotateY;
		}
		break;
	case 5: // 後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2WaveFinishedGapTimerMax);
		}
		break;
	case 6: // 見た目を戻す.
		destinationHalberdTransform_.translate = Easing(kPhase2SpinningHalberdSpinGapPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kPhase2SpinningHalberdSpinGapRotate, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = Easing(preTransform_.rotate.y + kPhase2SpinningSpinGapRotateY, preTransform_.rotate.y, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
		}
		break;
	}


	attackTempCollider_->SetTransform(attackTempTransform_);
	attackTempTransform_ = destinationHalberdTransform_;
	if (currentAttackPhase <= 4 && currentAttackPhase >= 2) {
		// 回転時ハルバードに当たり判定を出す.
		transform_.translate += move * deltaTime_ * DifficultyManager::GetInstance()->GetSpeedMagnification() * dopamineSpeed_;
		attackTempCollider_->SetSize({ kBasicHalberdColliderSize.x + 2.0f,kBasicHalberdColliderSize.y,kBasicHalberdColliderSize.z });
		attackTempTransform_.translate.y = -1.8f;
		CollisionManager::GetInstance()->AddColliderList(attackTempCollider_.get());
		attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	}

	attackTempCollider_->DrawCollider();
}

void Boss::Phase2PowerSlasherInitialize() {
	kMaxAttackTimer = kPhase2PowerSlasherStartGapTimerMax;
	attackTempTransform_.translate = { 1.0f,0.0f,0.0f };
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);
	attackTempCollider_->SetColliderType(ColliderType::kBox);
	attackTempCollider_->SetDimensionType(ColliderDimensionType::k3D);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack));
	attackTempCollider_->SetCollisionMask(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer));
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	halberdTransform_.SetParent(&transform_);
	powerSlasherHalberdCenter_.Initialize();
	powerSlasherHalberdCenter_.SetParent(&transform_);
	halberdTransform_.SetParent(&powerSlasherHalberdCenter_);
	attackTempTransform_.SetParent(&powerSlasherHalberdCenter_);
	attackTempCollider_->SetDamage(25.0f);
	attackTempCollider_->SetDamageCoolTime(3.0f);
}

void Boss::Phase2PowerSlasherUpdate() {
	Vector3 lenght;
	attackTempTransform_ = halberdTransform_;
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);

	Vector3 move = { 0.0f,0.0f,-kPhase2PowerSlasherSpeed };
	Vector2 direction = { 0.0f,0.0f };

	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix({ 0.0f,transform_.rotate.y,0.0f });
	move = rotateMatrix.TransformNomal(move);

	switch (currentAttackPhase) {
	case 0: // ハルバードを構える.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kPhase2PowerSlasherHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kPhase2PowerSlasherHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(0.0f, kPhase2PowerSlasherModelStartRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2PowerSlasherStayTimerMax);
		}
		break;
	case 1: // 前隙.
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2PowerSlasherStayBlankTimerMax);
		}
		break;
	case 2: // 前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			lenght = targetTransform_->translate - transform_.translate;
			// プレイヤーの位置によって攻撃が変わる.
			if (lenght.Length() < kPhase2PowerSlasherNearSlashRadius) {
				// 敵に近い位置なら2に遷移.
				SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f, 0.25f, kSoundEffect);
				SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
				NextAttackPhase(kPhase2PowerSlasherDashToSlashTimerMax);
				powerSlasherRotateY_ = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
				powerSlasherDirection_ = Vector3(transform_.translate.x - targetTransform_->translate.x, 0.0f, transform_.translate.z - targetTransform_->translate.z);
				preTransform_ = transform_;
			} else {
				// 敵から離れた位置なら3に遷移.
				currentAttackPhase++;
				NextAttackPhase(kPhase2PowerSlasherDashTimerMax);
			}
		}
		break;
	case 3: // 攻撃しながら構えなおす.
		transform_.rotate.y = Easing(preTransform_.rotate.y, powerSlasherRotateY_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.translate = Easing(kPhase2PowerSlasherHalberdStartPos, kPhase2PowerSlasherHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kPhase2PowerSlasherHalberdStartRotate, kPhase2PowerSlasherHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		powerSlasherHalberdCenter_.rotate.y = Easing(0.0f, Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kPhase2PowerSlasherModelStartRotateY, kPhase2PowerSlasherModelFinishedRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		SlashEffectCreate(&attackTempTransform_, 3);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			// 7(後隙)に遷移.
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateDiffusionBullet(halberdTransform_, -powerSlasherDirection_.Normalize() * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(10.0f), 5);
			currentAttackPhase++;
			currentAttackPhase++;
			currentAttackPhase++;
			NextAttackPhase(kPhase2PowerSlasherStayTimerMax);
		}
		break;
	case 4:  // 突進をする.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		lenght = targetTransform_->translate - transform_.translate;
		// プレイヤーの位置によって攻撃の終わるタイミングが変わる.
		if (lenght.Length() < kPhase2PowerSlasherSlashRadius) {
			// 射程圏内に入ったら5に遷移.
			NextAttackPhase(kPhase2PowerSlasherDashToSlashTimerMax);
		}

		if (kPhase2PowerSlasherSlashRadius + transform_.translate.Length() > movingRadius_) {
			//場外に移動しそうになったら5に遷移.
			NextAttackPhase(kPhase2PowerSlasherDashToSlashTimerMax);
		}

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			// 突進時間が終了したら5に遷移.
			NextAttackPhase(kPhase2PowerSlasherDashToSlashTimerMax);
		}
		break;
	case 5: // 突進しながら構えなおす.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		destinationHalberdTransform_.translate = Easing(kPhase2PowerSlasherHalberdStartPos, kPhase2PowerSlasherHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kPhase2PowerSlasherHalberdStartRotate, kPhase2PowerSlasherHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2PowerSlasherDashToSlashTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f, 0.25f, kSoundEffect);
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
			direction.x = targetTransform_->translate.x - transform_.translate.x;
			direction.y = targetTransform_->translate.z - transform_.translate.z;
			powerSlasherRotateY_ = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
			powerSlasherDirection_ = Vector3(transform_.translate.x - targetTransform_->translate.x, 0.0f, transform_.translate.z - targetTransform_->translate.z);
			preTransform_ = transform_;
		}

		if (transform_.translate.Length() > movingRadius_) {
			AttackFinished();
			attackRequest_ = Attacks::kDown;
			AttackInitialize();
		}
		break;
	case 6: // 攻撃を行う.
		powerSlasherHalberdCenter_.rotate.y = Easing(0.0f, Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = Easing(preTransform_.rotate.y, powerSlasherRotateY_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kPhase2PowerSlasherModelStartRotateY, kPhase2PowerSlasherModelFinishedRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		SlashEffectCreate(&attackTempTransform_, 3);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateDiffusionBullet(halberdTransform_, -powerSlasherDirection_.Normalize() * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(10.0f), 5);
			NextAttackPhase(kPhase2PowerSlasherSlashStayTimerMax);
		}
		break;
	case 7: // 後隙(3または6から遷移される).
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2PowerSlasherFinishedGapTimerMax);
		}
		break;
	case 8: // 見た目を戻す.
		powerSlasherHalberdCenter_.rotate.y = Easing(Radian(180.0f), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kPhase2PowerSlasherModelFinishedRotateY, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.translate = Easing(kPhase2PowerSlasherHalberdAttackPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kPhase2PowerSlasherHalberdAttackRotate, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			halberdTransform_.SetParent(&transform_);
		}
		break;
	}

	if (currentAttackPhase <= 6 && currentAttackPhase >= 3) {
		// 突進時や攻撃時にはハルバードに当たり判定を作る.
		attackTempCollider_->SetSize({ kBasicHalberdColliderSize.x + 2.0f,kBasicHalberdColliderSize.y,kBasicHalberdColliderSize.z });
		attackTempTransform_.translate.y = -1.8f;
		CollisionManager::GetInstance()->AddColliderList(attackTempCollider_.get());
		attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	}

	attackTempCollider_->SetTransform(attackTempTransform_);
	attackTempCollider_->DrawCollider();
}

void Boss::Phase2FangAttackInitialize() {
	//AttackFinished();

	kMaxAttackTimer = kPhase2FangAttackStartGapTimerMax;
}

void Boss::Phase2FangAttackUpdate() {
	Transform effectTransform;
	switch (currentAttackPhase) {
	case 0: // 上昇しながらハルバードを前に構える.
		transform_.translate.y = Easing(kBasicPositionY, kPhase2FangAttackAnimPositionY / 2.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kPhase2FangAttackHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kPhase2FangAttackHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2FangAttackStartGapTimerMax);
			halberdTransform_.SetParent(&modelTransform_);
		}
		break;
	case 1: // 残りの上昇.
		transform_.translate.y = Easing(kPhase2FangAttackAnimPositionY / 2.0f, kPhase2FangAttackAnimPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.x = Easing(kPhase2FangAttackHalberdStartRotate.x, preTransform_.rotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kPhase2FangAttackHalberdPos, kPhase2FangAttackHalberdSpinPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2FangAttackSpinTimerMax);
			preTransform_ = transform_;
		}
		break;
	case 2: // その場で回転.
		transform_.rotate.x = Easing(preTransform_.rotate.x, preTransform_.rotate.x - Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2FangAttackAttackTimerMax);
		}
		break;
	case 3: // 攻撃態勢に入りながら急降下.
		effectTransform.Initialize();
		effectTransform.SetParent(&transform_);
		effectTransform.translate.z = -4.0f;
		effectTransform.translate.y = 2.0f;
		SlashEffectCreate(&effectTransform, 3);
		transform_.translate.y = Easing(kPhase2FangAttackAnimPositionY, kPhase2FangAttackAttackPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		transform_.rotate.x = Easing(preTransform_.rotate.x, kPhase2FangAttackAttackRotateX, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kPhase2FangAttackHalberdSpinPos, kPhase2FangAttackHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2FangAttackAttackGapTimerMax);
			Phase2FangAttackFangCreate();

		}
		break;
	case 4: // 攻撃後の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kPhase2FangAttackFinishedGapTimerMax);
		}
		break;
	case 5: // 見た目を戻す.
		transform_.translate.y = Easing(kPhase2FangAttackAttackPositionY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		transform_.rotate.x = Easing(kPhase2FangAttackAttackRotateX, preTransform_.rotate.x, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.translate = Easing(kPhase2FangAttackHalberdAttackPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			halberdTransform_.SetParent(&transform_);
		}
		break;
	}
}

void Boss::Phase2FangAttackFangCreate() {
	float rotateY = transform_.rotate.y - Radian(90.0f);
	float lenght = Vector3(targetTransform_->GetWorldPosition() - transform_.GetWorldPosition()).Length();
	Transform newTransform;
	newTransform.Initialize();
	Vector2 center = { halberdTransform_.GetWorldPosition().x, halberdTransform_.GetWorldPosition().z };

	if (lenght <= kPhase2FangAttackRadius) {
		for (uint32_t j = 0; j < kPhase2FangAttackLoopCount; j++) {
			uint32_t maxCount = 4 * j;
			float rotateBlank = Random::GetInstance()->RandomFloat(0.1f, (360.0f / maxCount));

			for (uint32_t i = 0; i < maxCount; i++) {
				newTransform.translate.x = Rotate({ kPhase2FangAttackRadius * Easing(0.0f,1.0f,static_cast<float>(j),static_cast<float>(kPhase2FangAttackLoopCount),EaseType::kConstant),0.0f }, center, (i * (360.0f / maxCount) + rotateBlank)).x;
				newTransform.translate.z = Rotate({ kPhase2FangAttackRadius * Easing(0.0f,1.0f,static_cast<float>(j),static_cast<float>(kPhase2FangAttackLoopCount),EaseType::kConstant),0.0f }, center, (i * (360.0f / maxCount) + rotateBlank)).y;
				ProjectileManager::GetInstance()->CreateSpike(newTransform, 0, kCollisionEnemyAttack, 15.0f, 3.0f);
			}
		}
	} else {
		//ProjectileManager::GetInstance()->CreateDiffusionBullet(halberdTransform_, Vector3(-RadianToVector(rotateY).x, 0.0f, RadianToVector(rotateY).y) * 20.0f, BulletType::kSpike, kCollisionEnemyAttack, 15.0f, 3.0f,Radian(15.0f),3);
		ProjectileManager::GetInstance()->CreateBullet(halberdTransform_, Vector3(-RadianToVector(rotateY).x, 0.0f, RadianToVector(rotateY).y) * 10.0f, BulletType::kSlowSpike, kCollisionEnemyAttack, 15.0f, 3.0f);
	}
}

void Boss::SpecialAttackInitialize() {
	kMaxAttackTimer = kSpecialAttackWarpEnterTimerMax;

	specialAttackCount_ = 0;
}

void Boss::SpecialAttackUpdate() {
	std::vector<AttackData> specialAttackData;
	AttackData newAttackData;
	newAttackData.attackName = Attacks::kWarp;
	newAttackData.weight = 1.0f;
	newAttackData.continuousCount = 0;
	newAttackData.magnification = 1.0f;
	float random = 0.0f;
	switch (currentAttackPhase) {
	case 0: // ワープの始まり.
		transform_.scale = Easing({ 1.0f,1.0f,1.0f }, { 2.0f,0.0f,2.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		halberdLeft_->SetRotate(Easing(basicHalberdLeftRotate, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdRight_->SetRotate(Easing(basicHalberdRightRotate, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kSpecialAttackWarpFinishedTimerMax);
			transform_.translate = { 0.0f,kBasicPositionY,0.0f };
			//transform_.translate = GetMoveAnchorPointFind(60.0f);
		}

		break;
	case 1: // ワープの終わり.
		transform_.scale = Easing({ 0.0f,2.0f,0.0f }, { 1.0f,1.0f,1.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_special_attack", 1.0f, 0.25f, kSoundEffect);
			NextAttackPhase(kSpecialAttackChargeStartTimerMax);
		}
		break;
	case 2: // ワープの終わり.
		//transform_.scale = Easing(, { 1.0f,1.0f,1.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		for (uint32_t i = 0; i < 5; i++) {
			ParticleManager::GetInstance()->SpawnParticles("charge", transform_.GetWorldPosition());
		}
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kSpecialAttackChargeStartTimerMax);

		}
		break;
	case 3: // ワープの終わり.
		//transform_.scale = Easing(, { 1.0f,1.0f,1.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		LightManager::GetInstance()->GetDirectionalLightData()->intensity = Easing(0.15f, 10.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kSpecialAttackFlashTimerMax);

		}
		break;
	case 4: // ワープの終わり.
		//transform_.scale = Easing(, { 1.0f,1.0f,1.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		LightManager::GetInstance()->GetDirectionalLightData()->intensity = Easing(10.0f, 300.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_shine", 1.0f, 0.25f, kSoundEffect);
			NextAttackPhase(kSpecialAttackFlashTimerMax);
		}
		break;
	case 5: // ワープの終わり.
		//transform_.scale = Easing(, { 1.0f,1.0f,1.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		LightManager::GetInstance()->GetDirectionalLightData()->intensity = Easing(300.0f, 0.15f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {

			if (!isDownThreeWayShot_) {
				newAttackData.attackName = Attacks::kThreeWayWave;
				newAttackData.weight = 1.0f;
				newAttackData.continuousCount = 0;
				newAttackData.magnification = 1.0f;
				specialAttackData.push_back(newAttackData);
			}

			if (!isDownAutoHalberd_) {
				newAttackData.attackName = Attacks::kAutoHalberd;
				newAttackData.weight = 1.0f;
				newAttackData.continuousCount = 0;
				newAttackData.magnification = 1.0f;
				specialAttackData.push_back(newAttackData);
			}

			if (!isDownInfinitySlasher_) {
				newAttackData.attackName = Attacks::kInfinitySlasher;
				newAttackData.weight = 1.0f;
				newAttackData.continuousCount = 0;
				newAttackData.magnification = 1.0f;
				specialAttackData.push_back(newAttackData);
			}

			AttackFinished();
			if (specialAttackData.empty()) {
				attackRequest_ = Attacks::kWarp;
			} else {
				AttackSelect(specialAttackData);
			}
			//attackRequest_ = Attacks::kInfinitySlasher;
			AttackInitialize();
			//random = Random::GetInstance()->RandomFloat(1.0f, 4.0f);
			//if (random < 2.0f) {
			//	AttackFinished();
			//	attackRequest_ = Attacks::kThreeWayWave;
			//	AttackInitialize();
			//} else if (random < 3.0f) {
			//	AttackFinished();
			//	attackRequest_ = Attacks::kAutoHalberd;
			//	AttackInitialize();
			//} else {
			//	AttackFinished();
			//	attackRequest_ = Attacks::kInfinitySlasher;
			//	AttackInitialize();
			//}
		}
		break;
	}
}

void Boss::ThreeWayWaveInitialize() {
	waveSpinHalTransform_.Initialize();
	waveSpinHalTransform_.SetParent(&transform_);
	kMaxAttackTimer = kThreeWayWaveStartGapTimerMax;
	movingBulletTimer_ = 0.0f;
	movingBulletTargetPos = GetMoveAnchorPointFind(kPhase2MovingBulletAnchorRadius);
	halberdTransform_.SetParent(&modelTransform_);
	halberdLeft_->SetParent(&waveSpinHalTransform_);
	halberdRight_->SetParent(&waveSpinHalTransform_);
	isColliderActive_ = false;

	waveSpinHalRotateY_ = Random::GetInstance()->RandomFloat(Radian(-360.0f), Radian(360.0f));

	if (std::abs(waveSpinHalRotateY_) < Radian(180.0f)) {
		if (waveSpinHalRotateY_ >= 0.0f) {
			waveSpinHalRotateY_ += Radian(360.0f);
		} else {
			waveSpinHalRotateY_ += Radian(-360.0f);
		}
	}
}

void Boss::ThreeWayWaveUpdate() {
	switch (currentAttackPhase) {
	case 0: // ハルバードを上昇.
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kThreeWayWaveHalberdStayPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kThreeWayWaveHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(0.0f, Radian(360.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		halberdLeft_->SetPosition(Easing(kBasicHalberdLeftPos, kThreeWayWaveHalberdLeftStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdLeft_->SetRotate(Easing(basicHalberdLeftRotate, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdRight_->SetPosition(Easing(kBasicHalberdRightPos, kThreeWayWaveHalberdRightStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdRight_->SetRotate(Easing(basicHalberdRightRotate, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		waveSpinHalTransform_.rotate.y = Easing(0.0f, waveSpinHalRotateY_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		waveSpinHalTransform_.translate.y = Easing(0.0f, kThreeWayWaveSpinHalPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kThreeWayWaveStayTimerMax);
		}
		break;
	case 1: // ハルバードを上昇.
		destinationHalberdTransform_.rotate = Easing(kThreeWayWaveHalberdStartRotate, kThreeWayWaveHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kThreeWayWaveAttackTimerMax);
		}
		break;
	case 2: // 攻撃態勢に入りながら急降下.
		destinationHalberdTransform_.translate = Easing(kThreeWayWaveHalberdStayPos, kThreeWayWaveHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kThreeWayWaveHalAttackGapTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateWave(destinationHalberdTransform_, 25.0f, 1.0f, 2.0f, kCollisionEnemyAttack, 15.0f, 3.0f);
		}
		break;
	case 3: // 攻撃後の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kThreeWayWaveAttackTimerMax);
			isColliderActive_ = true;
		}
		break;
	case 4: // 攻撃後の後隙.
		halberdLeft_->SetRotateX(Easing(Radian(0.0f), kThreeWayWaveHalberdAttackRotate.x, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		halberdRight_->SetRotateX(Easing(Radian(0.0f), kThreeWayWaveHalberdAttackRotate.x, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kThreeWayWaveAttackTimerMax);
		}
		break;
	case 5: // 攻撃後の後隙.
		waveSpinHalTransform_.translate.y = Easing(kThreeWayWaveSpinHalPositionY, kThreeWayWaveSpinHalAttackPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kThreeWayWaveHalAttackGapTimerMax);
			if (std::abs(waveSpinHalRotateY_) > Radian(360.0f)) {
				if (waveSpinHalRotateY_ >= 0.0f) {
					waveSpinHalRotateY_ -= Radian(360.0f);
				} else {
					waveSpinHalRotateY_ -= Radian(-360.0f);
				}
			}
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateWave(halberdLeft_->GetTransform(), 25.0f, 1.0f, 2.0f, kCollisionEnemyAttack, 5.0f, 3.0f);
			ProjectileManager::GetInstance()->CreateWave(halberdRight_->GetTransform(), 25.0f, 1.0f, 2.0f, kCollisionEnemyAttack, 5.0f, 3.0f);

		}
		break;
	case 6: // 攻撃後の後隙.
		destinationHalberdTransform_.translate = Easing(kThreeWayWaveHalberdAttackRotate, kThreeWayWaveHalberdStayPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kThreeWayWaveHalberdAttackPos, kThreeWayWaveHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kThreeWayWaveStartGapTimerMax);
			preWaveSpinHalRotateY_ = waveSpinHalRotateY_;
			waveSpinHalRotateY_ += Radian(30.0f);
		}
		break;
	case 7: // 攻撃後の後隙.
		halberdLeft_->SetRotateX(Easing(kThreeWayWaveHalberdAttackRotate.x, Radian(0.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		halberdRight_->SetRotateX(Easing(kThreeWayWaveHalberdAttackRotate.x, Radian(0.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		waveSpinHalTransform_.rotate.y = Easing(preWaveSpinHalRotateY_, waveSpinHalRotateY_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		waveSpinHalTransform_.translate.y = Easing(kThreeWayWaveSpinHalAttackPositionY, kThreeWayWaveSpinHalPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kThreeWayWaveAttackTimerMax);
		}
		break;
	case 8: // 攻撃後の後隙.
		destinationHalberdTransform_.rotate = Easing(kThreeWayWaveHalberdStartRotate, kThreeWayWaveHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		destinationHalberdTransform_.translate = Easing(kThreeWayWaveHalberdStayPos, kThreeWayWaveHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kThreeWayWaveHalAttackGapTimerMax);
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateWave(destinationHalberdTransform_, 25.0f, 1.0f, 2.0f, kCollisionEnemyAttack, 15.0f, 3.0f);
		}
		break;
	case 9: // 攻撃後の後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			if (specialAttackCount_ <= kThreeWayWaveCountMax_) {
				NextAttackPhase(kThreeWayWaveAttackTimerMax);
				currentAttackPhase = 4;
				specialAttackCount_++;
			} else {
				NextAttackPhase(kThreeWayWaveFinishedGapTimerMax);
			}
		}
		break;
	case 10: // 見た目を戻す.
		destinationHalberdTransform_.translate = Easing(kThreeWayWaveHalberdAttackPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		destinationHalberdTransform_.rotate = Easing(kThreeWayWaveHalberdAttackPos, basicHalberdRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		halberdLeft_->SetRotateX(Easing(kThreeWayWaveHalberdAttackRotate.x, Radian(0.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		halberdRight_->SetRotateX(Easing(kThreeWayWaveHalberdAttackRotate.x, Radian(0.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));

		waveSpinHalTransform_.rotate.y = Easing(waveSpinHalRotateY_, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);
		waveSpinHalTransform_.translate.y = Easing(kThreeWayWaveSpinHalPositionY, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut);

		halberdLeft_->SetPosition(Easing(kThreeWayWaveHalberdLeftStartPos, kBasicHalberdLeftPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdLeft_->SetRotate(Easing({ 0.0f,0.0f,0.0f }, basicHalberdLeftRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdRight_->SetPosition(Easing(kThreeWayWaveHalberdRightStartPos, kBasicHalberdRightPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));
		halberdRight_->SetRotate(Easing({ 0.0f,0.0f,0.0f }, basicHalberdRightRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseInOut));

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			modelTransform_.rotate.y = 0.0f;
			halberdTransform_.SetParent(&transform_);
			damageAmountRecord_ = 0.0f;
		}
		break;
	}
}

void Boss::AutoHalberdInitialize() {
	hpGauge->SetBackColor({ 0.3f, 0.3f, 0.3f });
	hpGauge->SetColor({ 0.7f,0.7f,0.7f });
	autoHalberdStop_ = false;
	kMaxAttackTimer = kAutoHalberdStartGapTimerMax;
	autoHalberdAttack_ = Attacks::kWarp;
	autoHalberdAttackPhase_ = 0;
	halberdLeft_->AutoStart();
	halberdRight_->AutoStart();
	autoHalberdFinishTimer_ = 0.0f;
	//GameCamera::GetInstance()->SetTargetIsAutoHalAttack(true);
}

void Boss::AutoHalberdUpdate() {
	switch (autoHalberdAttackPhase_) {
	case 0:

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kAutoHalberdStartGapTimerMax);
			autoHalberdAttackPhase_++;
			currentAttackTimer_ = 0.0f;
			DistanceCheckUpdate();
			if (currentDistance_ == DistanceName::kFar) {
				AutoAttackSelect();
			} else if (currentDistance_ == DistanceName::kMiddle) {
				autoHalberdAttack_ = Attacks::kMovingShot;
			} else {
				autoHalberdAttack_ = Attacks::kWarp;
			}
			transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
			preTransform_ = transform_;

			currentAttackPhase = 0;
			attackTempCollider_->SetRadius(4.0f);

			if (autoHalberdFinishTimer_ >= kAutoHalberdFinishTimerMax) {
				autoHalberdAttackPhase_ = 2;
				halberdLeft_->SetIsFinished(true);
				halberdRight_->SetIsFinished(true);
				return;
			}

			(this->*pInitializeFunc[static_cast<size_t>(autoHalberdAttack_)])();
		}
		break;
	case 1:
		autoHalberdFinishTimer_ += deltaTime_;
		(this->*pUpdateFunc[static_cast<size_t>(autoHalberdAttack_)])();
		break;
	case 2:

		if (!halberdLeft_->GetAutoMove() && !halberdRight_->GetAutoMove()) {
			NextAttackPhase(kAutoHalberdFinishGapTimerMax);
			autoHalberdAttackPhase_++;
		}
		break;
	case 3:
		hpGauge->SetBackColor(Easing({ 0.3f,0.3f,0.3f }, { 1.0f,0.1f,0.1f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant));
		hpGauge->SetColor(Easing({ 0.7f,0.7f,0.7f }, { 1.0f,1.0f,0.1f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant));
		halberdLeft_->SetPosition(Easing({ 0.0f,0.0f,0.0f }, kBasicHalberdLeftPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdRight_->SetPosition(Easing({ 0.0f,0.0f,0.0f }, kBasicHalberdRightPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			autoHalberdStop_ = true;
			damageAmountRecord_ = 0.0f;
			AttackFinished();
		}
		break;
	}
}

void Boss::AutoAttackSelect() {
	std::vector<std::pair<Attacks, float>> randomData;
	float weightMax = 0.0f;
	float selectNum;

	for (AttackData& data : autoHalberdAttackDatas_) {
		float weight = data.weight * std::pow(0.5f, static_cast<float>(data.continuousCount)) * data.magnification * 10000.0f;

		weightMax = weight + weightMax;

		randomData.push_back(std::pair<Attacks, float>(data.attackName, weight));
	}

	selectNum = Random::GetInstance()->RandomFloat(1.0f, weightMax);

	weightMax = 0.0f;

	for (std::pair<Attacks, float>& data : randomData) {
		weightMax += data.second;
		if (selectNum < weightMax) {
			autoHalberdAttack_ = data.first;
			break;
		}
	}
}

void Boss::InfinitySlasherInitialize() {
	infinitySlasherHalberdCenter_.Initialize();
	kMaxAttackTimer = kPowerSlasherStartGapTimerMax;
	attackTempTransform_.translate = { 1.0f,0.0f,0.0f };
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);
	attackTempCollider_->SetColliderType(ColliderType::kBox);
	attackTempCollider_->SetDimensionType(ColliderDimensionType::k3D);
	attackTempCollider_->SetCollisionAttribute(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionEnemyAttack));
	attackTempCollider_->SetCollisionMask(CollisionManager::GetInstance()->GetCollisionAttribute(kCollisionPlayer));
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	halberdTransform_.SetParent(&transform_);
	powerSlasherHalberdCenter_.Initialize();
	powerSlasherHalberdCenter_.SetParent(&transform_);
	halberdTransform_.SetParent(&powerSlasherHalberdCenter_);
	attackTempTransform_.SetParent(&powerSlasherHalberdCenter_);
	attackTempCollider_->SetDamage(25.0f);
	attackTempCollider_->SetDamageCoolTime(3.0f);
	infinitySlasherHalberdCenter_.rotate.y = transform_.rotate.y;
	infinitySlasherHalberdCenter_.translate.y = kBasicPositionY;
	halberdLeft_->SetParent(&infinitySlasherHalberdCenter_);
	halberdRight_->SetParent(&infinitySlasherHalberdCenter_);
	preInfinitySlasherHalCenterRotateY_ = infinitySlasherHalberdCenter_.rotate.y;
	infinitySlasherSlashCount_ = 0;
	infinitySlasherSlashPhaseCount_ = 0;
}

void Boss::InfinitySlasherUpdate() {
	Vector3 lenght;
	Transform bulletTransform;
	attackTempTransform_ = halberdTransform_;
	attackTempCollider_->SetDebugColor({ 1.0f,1.0f,1.0f,1.0f });
	attackTempCollider_->SetSize(kBasicHalberdColliderSize);

	Vector3 move = { 0.0f,0.0f,-kInfinitySlasherSpeed };
	Vector3 direction;
	Matrix4x4 rotateMatrix = Matrix4x4::MakeRotateMatrix({ 0.0f,transform_.rotate.y,0.0f });
	move = rotateMatrix.TransformNomal(move);

	switch (currentAttackPhase) {
	case 0: // ハルバードを構える.
		hpGauge->SetColor(Easing({ 1.0f,1.0f,0.1f }, { 0.7f,0.7f,0.7f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant));
		hpGauge->SetBackColor(Easing({ 1.0f,0.1f,0.1f }, { 0.3f,0.3f,0.3f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant));
		halberdLeft_->SetRotate(Easing(basicHalberdLeftRotate, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		halberdRight_->SetRotate(Easing(basicHalberdRightRotate, { 0.0f,0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		destinationHalberdTransform_.translate = Easing(basicHalberdPos, kInfinitySlasherHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(basicHalberdRotate, kInfinitySlasherHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(0.0f, kInfinitySlasherModelStartRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherHalRotateTimerMax);
		}
		break;
	case 1: // ハルバード大回転.
		infinitySlasherHalberdCenter_.rotate.y = Easing(preInfinitySlasherHalCenterRotateY_, preInfinitySlasherHalCenterRotateY_ + Radian(900.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		infinitySlasherHalberdCenter_.translate.y = Easing(kBasicPositionY, kInfinitySlasherHalCenterStartPosY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		halberdLeft_->SetPosition(Easing(kBasicHalberdLeftPos, kInfinitySlasherHalLeftStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdRight_->SetPosition(Easing(kBasicHalberdRightPos, kInfinitySlasherHalRightStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherStayBlankTimerMax);
		}
		break;
	case 2: // 前隙.
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherStayBlankTimerMax);
		}
		break;
	case 3: // 前隙2.
		halberdLeft_->SetRotate(Easing({ 0.0f,0.0f,0.0f }, { -Radian(180.0f),0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		halberdRight_->SetRotate(Easing({ 0.0f,0.0f,0.0f }, { -Radian(180.0f),0.0f,0.0f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		infinitySlasherHalberdCenter_.translate.y = Easing(kInfinitySlasherHalCenterStartPosY, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 0.25f, kSoundEffect);
			// 敵から離れた位置なら3に遷移.
			NextAttackPhase(kInfinitySlasherDashTimerMax);
			currentAttackPhase = 10;
			bulletTransform = halberdLeft_->GetTransform();
			bulletTransform.translate.y = 1.2f;
			ProjectileManager::GetInstance()->CreateDiffusionBullet(bulletTransform, Vector3(0.0f, 0.0f, 1.0f) * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(45.0f), 8);
			bulletTransform = halberdRight_->GetTransform();
			bulletTransform.translate.y = 1.2f;
			ProjectileManager::GetInstance()->CreateDiffusionBullet(bulletTransform, Vector3(0.0f, 0.0f, 1.0f) * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(45.0f), 8);
		}
		break;
		//==========================================================================================================================================
	case 10:  // 突進をする.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		lenght = targetTransform_->translate - transform_.translate;
		// プレイヤーの位置によって攻撃の終わるタイミングが変わる.
		if (lenght.Length() < kInfinitySlasherSlashRadius) {
			// 射程圏内に入ったら11に遷移.
			NextAttackPhase(kInfinitySlasherDashToSlashTimerMax);
		}

		if (kInfinitySlasherSlashRadius + transform_.translate.Length() > movingRadius_) {
			//場外に移動しそうになったら11に遷移.
			NextAttackPhase(kInfinitySlasherDashToSlashTimerMax);
		}

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			// 突進時間が終了したら11に遷移.
			NextAttackPhase(kInfinitySlasherDashToSlashTimerMax);
		}

		if ((transform_.GetWorldPosition() - halberdLeft_->GetWorldPosition()).Length() <= kInfinitySlasherHalberdJugdeRadius ||
			(transform_.GetWorldPosition() - halberdRight_->GetWorldPosition()).Length() <= kInfinitySlasherHalberdJugdeRadius) {
			DeltaTime::GetInstance()->SetHitStop(0.5f);
			isDownInfinitySlasher_ = true;
			AttackFinished();
			halberdLeft_->SetParent(&transform_);
			halberdRight_->SetParent(&transform_);
			halberdTransform_.SetParent(&transform_);
			attackRequest_ = Attacks::kSuperDown;
			AttackInitialize();
		}
		break;
	case 11: // 突進しながら構えなおす.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		destinationHalberdTransform_.translate = Easing(kInfinitySlasherHalberdStartPos, kInfinitySlasherHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kInfinitySlasherHalberdStartRotate, kInfinitySlasherHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f, 0.25f, kSoundEffect);
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
			NextAttackPhase(kInfinitySlasherDashToSlashTimerMax);
		}
		break;
	case 12: // 攻撃を行う.
		powerSlasherHalberdCenter_.rotate.y = Easing(0.0f, Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kInfinitySlasherModelStartRotateY, kInfinitySlasherModelFinishedRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		SlashEffectCreate(&attackTempTransform_, 3);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherSlashStayTimerMax);
		}
		break;
	case 13: // 後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			if (infinitySlasherSlashCount_ >= kInfinitySlasherSlashCountMax - 1) {
				NextAttackPhase(kInfinitySlasherFinishedGapTimerMax);
				currentAttackPhase = 20;
			} else {
				NextAttackPhase(kInfinitySlasherFinishedGapTimerMax);
				infinitySlasherSlashCount_++;
				// 突進回数が規定値以下なら15に移行.
			}
		}
		break;
	case 14: // ハルバードを構える.
		destinationHalberdTransform_.translate = Easing(kInfinitySlasherHalberdAttackPos, kInfinitySlasherHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kInfinitySlasherHalberdAttackRotate, kInfinitySlasherHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		powerSlasherHalberdCenter_.rotate.y = Easing(Radian(180.0f), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kInfinitySlasherModelFinishedRotateY, kInfinitySlasherModelStartRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherStayTimerMax);
		}
		break;
	case 15: // 前隙.
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherStayTimerMax);
		}
		break;
	case 16: // 前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherDashTimerMax);
			currentAttackPhase = 10;
		}
		break;
		//==========================================================================================================================================
	case 20: // ハルバードを構える.
		destinationHalberdTransform_.translate = Easing(kInfinitySlasherHalberdAttackPos, kInfinitySlasherHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kInfinitySlasherHalberdAttackRotate, kInfinitySlasherHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		powerSlasherHalberdCenter_.rotate.y = Easing(Radian(180.0f), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kInfinitySlasherModelFinishedRotateY, kInfinitySlasherModelStartRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = atan2(transform_.translate.x, transform_.translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherStayTimerMax);
		}
		break;
	case 21: // 前隙.
		transform_.rotate.y = atan2(transform_.translate.x, transform_.translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherStayTimerMax);
		}
		break;
	case 22: // 前隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherDashTimerMax);
		}
		break;
	case 23:  // 突進をする.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		if (transform_.translate.Length() < kInfinitySlasherSlashRadius + 5.0f) {
			//原点に近くなったら24に遷移.
			NextAttackPhase(kInfinitySlasherDashToSlashTimerMax);
		}
		break;
	case 24: // 突進しながら構えなおす.
		transform_.translate += move * deltaTime_ * difficultyMagnificationTime * dopamineSpeed_;
		destinationHalberdTransform_.translate = Easing(kInfinitySlasherHalberdStartPos, kInfinitySlasherHalberdAttackPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kInfinitySlasherHalberdStartRotate, kInfinitySlasherHalberdAttackRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_near_attack", 1.0f, 0.25f, kSoundEffect);
			SoundManager::GetInstance()->SoundPlay("snd_attack_wind", 1.0f, 0.25f, kSoundEffect);
			NextAttackPhase(kInfinitySlasherDashToSlashTimerMax);
			direction.x = targetTransform_->translate.x - transform_.translate.x;
			direction.y = targetTransform_->translate.z - transform_.translate.z;
			powerSlasherRotateY_ = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
			powerSlasherDirection_ = Vector3(transform_.translate.x - targetTransform_->translate.x, 0.0f, transform_.translate.z - targetTransform_->translate.z);
			preTransform_ = transform_;
		}
		break;
	case 25: // 攻撃を行う.
		powerSlasherHalberdCenter_.rotate.y = Easing(0.0f, Radian(180.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kInfinitySlasherModelStartRotateY, kInfinitySlasherModelFinishedRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = Easing(preTransform_.rotate.y, powerSlasherRotateY_, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		SlashEffectCreate(&attackTempTransform_, 3);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			SoundManager::GetInstance()->SoundPlay("snd_bullet_shot", 1.0f, 0.25f, kSoundEffect);
			ProjectileManager::GetInstance()->CreateDiffusionBullet(halberdTransform_, -powerSlasherDirection_.Normalize() * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(10.0f), 5);
			NextAttackPhase(kInfinitySlasherSlashStayTimerMax);
			if (infinitySlasherSlashPhaseCount_ >= kInfinitySlasherSlashPhaseCountMax - 1) {
				NextAttackPhase(kInfinitySlasherHalRotateTimerMax);
				preInfinitySlasherHalCenterRotateY_ = infinitySlasherHalberdCenter_.rotate.y;
				currentAttackPhase = 40;

				//NextAttackPhase(kInfinitySlasherFinishedGapTimerMax);
			} else {
				infinitySlasherSlashPhaseCount_++;
				NextAttackPhase(kInfinitySlasherSlashStayTimerMax);
				preInfinitySlasherHalCenterRotateY_ = infinitySlasherHalberdCenter_.rotate.y;
			}
		}
		break;
	case 26: // 後隙.
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherFinishedGapTimerMax);
		}
		break;
	case 27: // ハルバードを構える.
		destinationHalberdTransform_.translate = Easing(kInfinitySlasherHalberdAttackPos, kInfinitySlasherHalberdStartPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kInfinitySlasherHalberdAttackRotate, kInfinitySlasherHalberdStartRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		powerSlasherHalberdCenter_.rotate.y = Easing(Radian(180.0f), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kInfinitySlasherModelFinishedRotateY, kInfinitySlasherModelStartRotateY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		transform_.rotate.y = atan2(transform_.translate.x, transform_.translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherHalRotateTimerMax);
		}
		break;
	case 28: // ハルバード大回転.
		infinitySlasherHalberdCenter_.rotate.y = Easing(preInfinitySlasherHalCenterRotateY_, preInfinitySlasherHalCenterRotateY_ + Radian(90.0f), currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		infinitySlasherHalberdCenter_.translate.y = Easing(0.0f, kInfinitySlasherHalCenterStartPosY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);

		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherStayBlankTimerMax);
		}
		break;
	case 29: // 前隙.
		transform_.rotate.y = atan2(transform_.translate.x - targetTransform_->translate.x, transform_.translate.z - targetTransform_->translate.z);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherStayBlankTimerMax);
		}
		break;
	case 30: // 前隙2.
		infinitySlasherHalberdCenter_.translate.y = Easing(kInfinitySlasherHalCenterStartPosY, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn);

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			// 敵から離れた位置なら3に遷移.
			SoundManager::GetInstance()->SoundPlay("snd_wave_shot", 1.0f, 0.25f, kSoundEffect);
			NextAttackPhase(kInfinitySlasherDashTimerMax);
			currentAttackPhase = 10;
			infinitySlasherSlashCount_ = 0;
			bulletTransform = halberdLeft_->GetTransform();
			bulletTransform.translate.y = 1.2f;
			ProjectileManager::GetInstance()->CreateDiffusionBullet(bulletTransform, Vector3(0.0f, 0.0f, 1.0f) * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(45.0f), 8);
			bulletTransform = halberdRight_->GetTransform();
			bulletTransform.translate.y = 1.2f;
			ProjectileManager::GetInstance()->CreateDiffusionBullet(bulletTransform, Vector3(0.0f, 0.0f, 1.0f) * 30.0f, BulletType::kNormal, kCollisionEnemyAttack, 10.0f, 3.0f, Radian(45.0f), 8);
		}
		break;
		//==========================================================================================================================================
	case 40: // 後隙(3または6から遷移される).
		infinitySlasherHalberdCenter_.rotate.y = Easing(preInfinitySlasherHalCenterRotateY_, transform_.rotate.y, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		infinitySlasherHalberdCenter_.translate.y = Easing(kInfinitySlasherHalCenterStartPosY, kBasicPositionY, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		halberdLeft_->SetPosition(Easing(kInfinitySlasherHalLeftStartPos, kBasicHalberdLeftPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdRight_->SetPosition(Easing(kInfinitySlasherHalRightStartPos, kBasicHalberdRightPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut));
		halberdLeft_->SetRotate(Easing({ -Radian(180.0f),0.0f,0.0f }, basicHalberdLeftRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));
		halberdRight_->SetRotate(Easing({ -Radian(180.0f),0.0f,0.0f }, basicHalberdRightRotate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseIn));

		if (currentAttackTimer_ >= kMaxAttackTimer) {
			NextAttackPhase(kInfinitySlasherFinishedGapTimerMax);
			preTransform_ = powerSlasherHalberdCenter_;
		}
		break;
	case 41: // 見た目を戻す.
		hpGauge->SetBackColor(Easing({ 0.3f,0.3f,0.3f },  { 1.0f,0.1f,0.1f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant));
		hpGauge->SetColor(Easing({ 0.7f, 0.7f, 0.7f }, { 1.0f,1.0f,0.1f }, currentAttackTimer_, kMaxAttackTimer, EaseType::kConstant));
		powerSlasherHalberdCenter_.rotate.y = Easing(Radian(180.0f), 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		powerSlasherHalberdCenter_.translate = Easing(preTransform_.translate, transform_.translate, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		modelTransform_.rotate.y = Easing(kInfinitySlasherModelFinishedRotateY, 0.0f, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.translate = Easing(kInfinitySlasherHalberdAttackPos, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		destinationHalberdTransform_.rotate = Easing(kInfinitySlasherHalberdAttackRotate, basicHalberdPos, currentAttackTimer_, kMaxAttackTimer, EaseType::kEaseOut);
		if (currentAttackTimer_ >= kMaxAttackTimer) {
			AttackFinished();
			halberdLeft_->SetParent(&transform_);
			halberdRight_->SetParent(&transform_);
			halberdTransform_.SetParent(&transform_);
			damageAmountRecord_ = 0.0f;
		}
		break;
	}

	//if (currentAttackPhase <= 6 && currentAttackPhase >= 3) {
		// 突進時や攻撃時にはハルバードに当たり判定を作る.
	attackTempCollider_->SetSize({ kBasicHalberdColliderSize.x + 2.0f,kBasicHalberdColliderSize.y,kBasicHalberdColliderSize.z });
	attackTempTransform_.translate.y = -1.8f;
	CollisionManager::GetInstance()->AddColliderList(attackTempCollider_.get());
	attackTempCollider_->SetDebugColor({ 1.0f,0.0f,0.0f,1.0f });
	//}

	attackTempCollider_->SetTransform(attackTempTransform_);
	attackTempCollider_->DrawCollider();
}
