#include "GameScene.h"
#include "../Satlib.h"
#include "../ForwardEnemy.h"
#include "../ForwardEnemyApproachPhase.h"
#include "../ForwardEnemyApproachHomingPhase.h"

GameScene::~GameScene() {
	delete player_;
	delete skydome_;

	for (BaseBullet* bullet : bullets_) {
		delete bullet;
	}
	bullets_.clear();
}

void GameScene::Initialize() {
	//TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	//TextureManager::GetInstance()->RegisterTexture("player_texture", "Resource/kari_texture/donut.png");
	//TextureManager::GetInstance()->RegisterTexture("bullet_texture", "Resource/kari_texture/bullet.png");
	//TextureManager::GetInstance()->RegisterTexture("enemy_bullet_texture", "Resource/kari_texture/enemy_bullet.png");
	//TextureManager::GetInstance()->RegisterTexture("enemy_texture", "Resource/kari_texture/kari_musikera.png");
	TextureManager::GetInstance()->RegisterTexture("reticle", "Resource/kari_texture/reticle.png");
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
	player_->SetGameScene(this);
	//;

	RegisterGlobalVariables();

	SpawnEnemy({ 10.0f,50.0f,100.0f },new ForwardEnemyApproachPhase());

	player_->SetRailCameraController(railCameraController_);

	skydome_ = new Skydome();
	skydome_->Initialize();

	LoadEnemyPopData();

	lockOn_ = new LockOn();
	lockOn_->Initialize();

	isWait_ = false;
	waitTimer_ = 0;
}

void GameScene::Update() {
	BulletRemoveCheck();
	EnemyRemoveCheck();
#ifdef _DEBUG
	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}
#endif // _DEBUG
	//UpdateEnemyPopCommands();

	ApplyGlobalVariables();

	for (std::shared_ptr<BaseEnemy> enemy : enemies_) {
		enemy->Update();
	}


	railCameraController_->Update();
	railCameraController_->SetTargetMatrix(player_->GetTransform().GetAffineMatrix());

	player_->Update();
	player_->SetIsLockOn(lockOn_->GetIsLockOn());
	player_->SetTarget(lockOn_->GetTarget());

	for (BaseBullet* bullet : bullets_) {
		bullet->Update();
	}

	std::list<std::weak_ptr<BaseEnemy>> enemies;

	for (std::shared_ptr<BaseEnemy> enemy : enemies_) {
		std::weak_ptr<BaseEnemy> weakEnemy = enemy;

		enemies.push_back(weakEnemy);
	}

	lockOn_->Update(player_,enemies);

	Camera::GetInstance()->Update();

	CheckAllCollision();
}

void GameScene::CheckAllCollision() {
	CollisionManager* manager = CollisionManager::GetInstance();
	manager->ClearColliderList();
	manager->AddColliderList(player_);

	for (std::shared_ptr<BaseEnemy> enemy : enemies_) {
		manager->AddColliderList(enemy.get());
	}

	for (BaseBullet* bullet : bullets_) {
		manager->AddColliderList(bullet);
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

void GameScene::UpdateEnemyPopCommands(){
	if (isWait_) {
		waitTimer_--;
		if (waitTimer_ <= 0) {
			isWait_ = false;
		}
		return;
	}

	std::string line;

	while(getline(enemyPopCommands,line)){
		std::istringstream line_stream(line);

		std::string word;

		getline(line_stream, word, ',');

		if (word.find("/") == 0) {
			continue;
		}

		if (word.find("POP") == 0) {
			getline(line_stream, word, ',');
			float x = static_cast<float>(std::atof(word.c_str()));

			getline(line_stream, word, ',');
			float y = static_cast<float>(std::atof(word.c_str()));

			getline(line_stream, word, ',');
			float z = static_cast<float>(std::atof(word.c_str()));

			
			getline(line_stream, word, ',');

			ForwardEnemyBasePhase* phase_ = new ForwardEnemyApproachPhase();

			if (word == "HOMING") {
				phase_ = new ForwardEnemyApproachHomingPhase();
			};

			SpawnEnemy(Vector3(x,y,z),phase_);
		} else if (word.find("WAIT") == 0) {
			getline(line_stream, word, ',');
			int32_t waitTime = std::atoi(word.c_str());

			isWait_ = true;
			waitTimer_ = waitTime;
			break;
		}


	}
}

void GameScene::AddBullet(BaseBullet* baseBullet) {
	bullets_.push_back(baseBullet);
}

void GameScene::SpawnEnemy(const Vector3& position,ForwardEnemyBasePhase* type) {
	std::shared_ptr<BaseEnemy> newEnemy = std::make_shared<ForwardEnemy>();

	newEnemy->SetTarget(player_);
	//dynamic_cast<ForwardEnemy*>(newEnemy.get())->SetGameScene(this);
	newEnemy->Initialize(position);
	dynamic_cast<ForwardEnemy*>(newEnemy.get())->SetPhase(std::unique_ptr<ForwardEnemyBasePhase>(type));

	enemies_.push_back(newEnemy);
}

void GameScene::BulletRemoveCheck() {
	bullets_.remove_if([](BaseBullet* bullet) {
		if (!bullet->GetIsActive()) {
			delete bullet;
			return true;
		}
		return false;
		});
}

void GameScene::EnemyRemoveCheck(){
	enemies_.remove_if([](std::shared_ptr<BaseEnemy> enemy) {
		if (!enemy->GetIsAlive()) {
			enemy = nullptr;
			return true;
		}
		return false;
		});
}

void GameScene::Draw() {
	skydome_->Draw();

	groundModel_.Draw(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-10.0f }));

	player_->Draw();

	railCameraController_->Draw();

	for(std::shared_ptr<BaseEnemy> enemy : enemies_){
		enemy->Draw();
	}

	for (BaseBullet* bullet : bullets_) {
		bullet->Draw();
	}

	lockOn_->Draw();
}

void GameScene::RegisterGlobalVariables() {
	Player::RegisterGlobalVariables();
	//PlayerBullet::RegisterGlobalVariables();
}

void GameScene::ApplyGlobalVariables() {
	Player::ApplyGlobalVariables();
	//PlayerBullet::ApplyGlobalVariables();
}

void GameScene::LoadEnemyPopData() {
	std::ifstream file;
	file.open("./Resource/eneny_spawn/spawn_script.csv");
	assert(file.is_open());

	enemyPopCommands << file.rdbuf();
	file.close();
}
