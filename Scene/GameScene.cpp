#include "GameScene.h"
#include "../Satlib.h"
#include "../ProjectileManager.h"

GameScene::~GameScene() {
}

void GameScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("reticle", "Resource/reticle.png");
	TextureManager::GetInstance()->RegisterTexture("effect_plane", "Resource/EffectPlane/effect_plane.png");
	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");
	ModelManager::GetInstance()->RegisterObj("creeking", "Resource/creeking", "creeking.obj");
	ModelManager::GetInstance()->RegisterObj("ground", "Resource/Ground", "ground.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player_hovering_mode", "player.obj");
	ModelManager::GetInstance()->RegisterObj("player_right_arm", "Resource/player_hovering_mode/right_arm", "right_arm.obj");
	ModelManager::GetInstance()->RegisterObj("player_left_arm", "Resource/player_hovering_mode/left_arm", "left_arm.obj");
	ModelManager::GetInstance()->RegisterObj("player_head", "Resource/player_hovering_mode/head", "head.obj");
	ModelManager::GetInstance()->RegisterObj("hammer_of_justice", "Resource/Hammer", "hammer_of_justice_uv.obj");
	ModelManager::GetInstance()->RegisterObj("enemy", "Resource/enemy", "enemy.obj");

	Camera::GetInstance()->SetPosition({ 0.0f,2.0f,-30.0f });
	GameCamera::GetInstance()->Initialize();



	player_ = std::make_unique<Player>();
	player_->Initialize();

	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();


	boss_ = std::make_unique<Boss>();
	boss_->Initialize();
	boss_->SetTargetTransform(player_->GetTransform());

	ProjectileManager::GetInstance()->Initialize();

	Player::RegisterGlobalVariables();

	//testNum_ = 5;
}

void GameScene::Update() {
	Player::ApplyGlobalVariables();

	skydome_->Update();

	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		SceneManager::GetInstance()->ReloadScene();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
		Camera::GetInstance()->ChangeCameraMode();
	}

#ifdef _DEBUG

	ImGui::Begin("drawTest");
	ImGui::DragInt("num", &testNum_, 1.0f, 0, 2000);
	ImGui::End();

	ImGui::Begin("spriteTest");
	testRotate_ = Degree(testRotate_);

	ImGui::DragFloat("num", &testRotate_, 1.0f, -360.0f, 360.0f);
	ImGui::DragFloat("major", &majorRadius_, 0.5f, 0.0f, 10.0f);
	ImGui::DragFloat("minor", &minorRadius_, 0.5f, 0.0f, 10.0f);
	testRotate_ = Radian(testRotate_);
	ImGui::End();

#endif // _DEBUG


	player_->Update();

	boss_->Update();

	ProjectileManager::GetInstance()->Update();

	GameCamera::GetInstance()->Update();

	Camera::GetInstance()->Update();

	CheckAllCollisions();
}

void GameScene::Draw() {
	skydome_->Draw();
	ground_->Draw();

	Renderer::GetInstance()->SetBlendMode(BlendMode::kNormalCullNone);
	Renderer::GetInstance()->DrawBox(Transform::GetInitialValue({ 150.0f,50.0f,150.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "uvChecker", { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawBox(Transform::GetInitialValue({ 150.0f,50.0f,150.0f }, { 0.0f,Radian(45.0f),0.0f }, { 0.0f,0.0f,0.0f }), "uvChecker", { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->SetBlendMode(BlendMode::kNormal);

	//Renderer::GetInstance()->DrawSprite(Transform2D::GetTransformValue({ 1.0f,1.0f }, testRotate_, { 150.0f,50.0f },0.0f), "uvChecker", { 1.0f,1.0f,1.0f,1.0f });
	//Renderer::GetInstance()->DrawSphere(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }), "uvChecker", { 1.0f,1.0f,1.0f,1.0f });
	Renderer::GetInstance()->DrawTorus(Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }),majorRadius_,minorRadius_, "uvChecker", { 1.0f,1.0f,1.0f,1.0f });

	ProjectileManager::GetInstance()->Draw();

	player_->Draw();
	boss_->Draw();

	if (testNum_ > 50) {
		testNum_ = testNum_;
	}

	for (uint32_t i = 0; i < static_cast<uint32_t>(testNum_); i++) {
		Transform transformTest = Transform::GetInitialValue({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { (-static_cast<float>(testNum_) / 2.0f) + static_cast<float>(i),0.0f,0.0f });
		Vector3 startPos = { (-static_cast<float>(testNum_) / 2.0f) + static_cast<float>(i),0.0f,0.0f };
		Vector3 endPos = { (-static_cast<float>(testNum_) / 2.0f) + static_cast<float>(i),1.0f,0.0f };
		Renderer::GetInstance()->DrawSphereWireFrame(transformTest, {1.0f,1.0f,1.0f,1.0f});
	}
}

void GameScene::CheckAllCollisions() {
}
