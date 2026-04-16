#include "Camera.h"
#include "GameSystem.h"
#include "GameScene.h"


// オーディオ類.

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem* system = GameSystem::GetInstance();

	system->Initialize();

	Camera::GetInstance()->Initialize(static_cast<float>(Environment::GetInstance()->GetWindowSize().width), static_cast<float>(Environment::GetInstance()->GetWindowSize().height));

	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/

	GameScene* gameScene = new GameScene();
	gameScene->Initialize();

	// ウィンドウのxボタンが押されるまでループ.
	while (system->ProcessMessage()) {
		if (system->BeginFrame()) {
//
			/*=============================================================
			以下にゲームの更新処理を記述.
			=============================================================*/

			gameScene->Update();

			/*=============================================================
			以下にゲームの描画処理を記述.
			=============================================================*/
			system->DrawSetup();

			gameScene->Draw();

			system->Endframe();
		}
	}

	system->Finalize();

	return 0;
}
