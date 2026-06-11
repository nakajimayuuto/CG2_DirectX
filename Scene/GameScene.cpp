#include "GameScene.h"
#include "../Satlib.h"

GameScene::~GameScene(){
	delete player_;
}

void GameScene::Initialize() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker","Resource/uvChecker.png");

	player_ = new Player();
	player_->Initialize();
}

void GameScene::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_R)) {
		//SetWindowLongW(GameSystem::GetInstance()->GetHWND(), GWL_STYLE, WS_OVERLAPPEDWINDOW);
		//
		//SetWindowPos(
		//	GameSystem::GetInstance()->GetHWND(),
		//	HWND_TOP,
		//	windowRect.left,
		//	windowRect.top,
		//	windowRect.right - windowRect.left,
		//	windowRect.bottom - windowRect.top,
		//	SWP_FRAMECHANGED | SWP_SHOWWINDOW
		//);
		SceneManager::GetInstance()->ReloadScene();

	player_->Update();

	if (isParticleUpdate_) {

		ParticleManager::GetInstance()->Update();
	}

	Camera::GetInstance()->Update();
}

void GameScene::Draw() {
	player_->Draw();
}
