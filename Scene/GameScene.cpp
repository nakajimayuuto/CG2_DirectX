#include "GameScene.h"
#include "../Satlib.h"
#include "../ForwardEnemy.h"

GameScene::~GameScene() {
	delete player_;
	delete enemy_;
}

void GameScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("player_texture", "Resource/kari_texture/donut.png");
	TextureManager::GetInstance()->RegisterTexture("bullet_texture", "Resource/kari_texture/bullet.png");
	TextureManager::GetInstance()->RegisterTexture("enemy_bullet_texture", "Resource/kari_texture/enemy_bullet.png");
	TextureManager::GetInstance()->RegisterTexture("enemy_texture", "Resource/kari_texture/kari_musikera.png");

	player_ = new Player();
	player_->Initialize();

	enemy_ = new ForwardEnemy();
	enemy_->SetPlayer(player_);
	enemy_->Initialize({ 10.0f,0.0f,50.0f });

	RegisterGlobalVariables();
}

void GameScene::Update() {
#ifdef _DEBUG
	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}
#endif // _DEBUG
	ApplyGlobalVariables();

	player_->Update();

	if (enemy_) {
		enemy_->Update();
	}

	Camera::GetInstance()->Update();

	CheckAllCollision();
}	

void GameScene::CheckAllCollision() {
	Sphere sphereA;
	Sphere sphereB;
	Vector3 scale;

	const std::list<BaseBullet*>& playerBullets = player_->GetBullet();

	if (ForwardEnemy* forwardEnemy = dynamic_cast<ForwardEnemy*>(enemy_)) {
		const std::list<BaseBullet*>& enemyBullets = forwardEnemy->GetBullet();

#pragma region 自キャラと敵弾.
		sphereA.center = player_->GetWorldPosition();
		scale = player_->GetTransform().scale;
		sphereA.radius = std::min(std::min(scale.x,scale.y),scale.z);

		for (BaseBullet* bullet : enemyBullets) {
			sphereB.center = bullet->GetWorldPosition();
			scale = bullet->GetTransform().scale;
			sphereB.radius = std::min(std::min(scale.x, scale.y), scale.z);

			if (Collision::SphereToSphere(sphereA,sphereB)) {
				player_->OnCollision();
				bullet->OnCollision();
			}
		}
#pragma endregion

#pragma region 自弾と敵弾.
		for (BaseBullet* enemyBullet : enemyBullets) {
			sphereA.center = enemyBullet->GetWorldPosition();
			scale = enemyBullet->GetTransform().scale;
			sphereA.radius = std::min(std::min(scale.x, scale.y), scale.z);

			for (BaseBullet* playerBullet : playerBullets) {
				sphereB.center = playerBullet->GetWorldPosition();
				scale = playerBullet->GetTransform().scale;
				sphereB.radius = std::min(std::min(scale.x, scale.y), scale.z);

				if (Collision::SphereToSphere(sphereA, sphereB)) {
					enemyBullet->OnCollision();
					playerBullet->OnCollision();
				}
			}
		}
#pragma endregion
	}

#pragma region 自弾と敵キャラ.
	sphereA.center = enemy_->GetWorldPosition();
	scale = enemy_->GetTransform().scale;
	sphereA.radius = std::min(std::min(scale.x, scale.y), scale.z);

	for (BaseBullet* bullet : playerBullets) {
		sphereB.center = bullet->GetWorldPosition();
		scale = bullet->GetTransform().scale;
		sphereB.radius = std::min(std::min(scale.x, scale.y), scale.z);

		if (Collision::SphereToSphere(sphereA, sphereB)) {
			enemy_->OnCollision();
			bullet->OnCollision();
		}
	}
#pragma endregion
}

void GameScene::Draw() {
	player_->Draw();

	if (enemy_) {
		enemy_->Draw();
	}
}

void GameScene::RegisterGlobalVariables() {
	Player::RegisterGlobalVariables();
	//PlayerBullet::RegisterGlobalVariables();
}

void GameScene::ApplyGlobalVariables() {
	Player::ApplyGlobalVariables();
	//PlayerBullet::ApplyGlobalVariables();
}
