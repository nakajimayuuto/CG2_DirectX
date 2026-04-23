#include "Camera.h"
#include "GameSystem.h"
#include "SceneManager.h"


// オーディオ類.

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	D3DResourceLeakChecker resourceLeakChecker;

	GameSystem* system = new GameSystem;

	system->Initialize();

	//Camera::GetInstance()->Initialize(static_cast<float>(Environment::GetInstance()->GetWindowSize().width), static_cast<float>(Environment::GetInstance()->GetWindowSize().height));

	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/


	//SceneManager::GetInstance()->Initialize();

	// ウィンドウのxボタンが押されるまでループ.
	while (system->ProcessMessage()) {
		if (system->BeginFrame()) {
//
			/*=============================================================
			以下にゲームの更新処理を記述.
			=============================================================*/

			//SceneManager::GetInstance()->Update();

			/*=============================================================
			以下にゲームの描画処理を記述.
			=============================================================*/
			system->DrawSetup();

			//SceneManager::GetInstance()->Draw();

			system->EndFrame();
		}
	}

	system->Finalize();

	system = nullptr;

	delete system;

	return 0;
}
