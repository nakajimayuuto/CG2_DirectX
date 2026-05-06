#include "TitleScene.h"

void TitleScene::Initialize(){
	ModelManager::GetInstance()->RegisterObj("title", "Resource/Title", "Title.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player", "player.obj");

	titleModel_.Initialize(ModelManager::GetInstance()->GetModelInfo("title"));
	titleTransform_.Initialize();
	titleTransform_.translate = { 0.0f,2.0f,0.0f };
	titleTransform_.rotate.x = Radian(-90.0f);

	playerModel_.Initialize(ModelManager::GetInstance()->GetModelInfo("player"));
	playerTransform_.Initialize();
	playerTransform_.rotate.y = Radian(30.0f);

	Camera::GetInstance()->Initialize();
	Camera::GetInstance()->SetPosition({0.0f,0.0f,-30.0f});
}

void TitleScene::Update(){
	if (InputManager::GetInstance()->TriggerKey(DIK_SPACE)) {
		SceneManager::GetInstance()->ChengeScene(SceneName::kGameScene);
	}

	Camera::GetInstance()->Update();
}

void TitleScene::Draw(){
	titleModel_.Draw(titleTransform_);
	playerModel_.Draw(playerTransform_);
}
