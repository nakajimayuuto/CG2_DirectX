#include "GameScene.h"
#include "../Satlib.h"
#include "../ForwardEnemy.h"

GameScene::~GameScene() {
	delete player_;
	delete enemy_;
	delete skydome_;
}

void GameScene::Initialize() {
	//TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	//TextureManager::GetInstance()->RegisterTexture("player_texture", "Resource/kari_texture/donut.png");
	//TextureManager::GetInstance()->RegisterTexture("bullet_texture", "Resource/kari_texture/bullet.png");
	//TextureManager::GetInstance()->RegisterTexture("enemy_bullet_texture", "Resource/kari_texture/enemy_bullet.png");
	//TextureManager::GetInstance()->RegisterTexture("enemy_texture", "Resource/kari_texture/kari_musikera.png");
	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player", "player.obj");
	ModelManager::GetInstance()->RegisterObj("enemy", "Resource/shield_enemy", "shield_enemy.obj");
	ModelManager::GetInstance()->RegisterObj("enemy_bullet", "Resource/death_particle", "death_particle.obj");
	ModelManager::GetInstance()->RegisterObj("player_bullet", "Resource/bullet", "bullet.obj");
	ModelManager::GetInstance()->RegisterObj("ground", "Resource/Ground", "ground.obj");

	groundModel_.Initialize(ModelManager::GetInstance()->GetModelInfo("ground"));

	railCameraController_ = new RailCameraController();
	railCameraController_->Initialize(Transform::GetInitialValue());

	player_ = new Player();
	player_->Initialize({ 0.0f,0.0f,50.0f });
	player_->SetParent(&railCameraController_->GetTransform());

	enemy_ = new ForwardEnemy();
	enemy_->SetPlayer(player_);
	enemy_->Initialize({ 10.0f,0.0f,50.0f });

	RegisterGlobalVariables();

	skydome_ = new Skydome();
	skydome_->Initialize();
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

	if (enemy_) {
		enemy_->Update();
	}

	railCameraController_->Update();

	player_->Update();

	Camera::GetInstance()->Update();

	CheckAllCollision();
}

void GameScene::CheckAllCollision() {
	CollisionManager* manager = CollisionManager::GetInstance();
	manager->ClearColliderList();
	manager->AddColliderList(player_);
	manager->AddColliderList(enemy_);

	const std::list<BaseBullet*>& playerBullets = player_->GetBullet();

	for (BaseBullet* bullet : playerBullets) {
		manager->AddColliderList(bullet);
	}

	if (ForwardEnemy* forwardEnemy = dynamic_cast<ForwardEnemy*>(enemy_)) {
		const std::list<BaseBullet*>& enemyBullets = forwardEnemy->GetBullet();
		for (BaseBullet* bullet : enemyBullets) {
			manager->AddColliderList(bullet);
		}
	}

	manager->CheckAllCollision();
}

void GameScene::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {
	if (
		((colliderA->GetCollisionAttribute() ^ colliderB->GetCollisionMask()) != 0xFFFFFFFF) ||
		((colliderB->GetCollisionAttribute() ^ colliderA->GetCollisionMask()) != 0xFFFFFFFF)
		) {
		return;
	}

	Sphere sphereA;
	Sphere sphereB;
	sphereA.center = colliderA->GetWorldPosition();
	sphereA.radius = colliderA->GetRadius();

	sphereB.center = colliderB->GetWorldPosition();
	sphereB.radius = colliderB->GetRadius();

	if (Collision::SphereToSphere(sphereA, sphereB)) {
		colliderA->OnCollision();
		colliderB->OnCollision();
	}
}

void GameScene::Draw() {
	skydome_->Draw();

	groundModel_.Draw(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-10.0f }));

	player_->Draw();

	railCameraController_->Draw();

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
