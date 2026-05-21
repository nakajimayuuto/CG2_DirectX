#include "GameScene.h"
#include "../Satlib.h"

void GameScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker","Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("monsterBall","Resource/monsterBall.png");
	model_.Initialize(TextureManager::GetInstance()->GetTextureInfo("uvChecker"));
	transform_.Initialize();
}

void GameScene::Update() {
	ImGui::Begin("Triangle");

	Vector3 imRotate = Degree(transform_.rotate);
	ImGui::DragFloat3("rotate", reinterpret_cast<float*>(&imRotate.x),1.0f,-360.0f,360.0f);
	transform_.rotate = Radian(imRotate);

	ImGui::End();

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	model_.Draw(transform_);
}