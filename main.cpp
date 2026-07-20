#include "Satlib.h"


// オーディオ類.

void LoadDatas() {
	// 使用するテクスチャの読み込み.

	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player", "player.obj");
	ModelManager::GetInstance()->RegisterObj("plane", "Resource", "plane.obj");


	// 使用するテクスチャの読み込み.
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("monsterBall", "Resource/monsterBall.png");
	TextureManager::GetInstance()->RegisterTexture("effect_triangle", "Resource/effect_triangle.png");
	TextureManager::GetInstance()->RegisterTexture("window_back", "Resource/Window/window_back.png");
	TextureManager::GetInstance()->RegisterTexture("window_mask", "Resource/Window/window_mask.png");
	TextureManager::GetInstance()->RegisterTexture("window_s_l_back", "Resource/Window/window_square_large_back.png");
	TextureManager::GetInstance()->RegisterTexture("window_s_l_mask", "Resource/Window/window_square_large_mask.png");
	TextureManager::GetInstance()->RegisterTexture("window_s_s_back", "Resource/Window/window_square_small_back.png");
	TextureManager::GetInstance()->RegisterTexture("window_s_s_mask", "Resource/Window/window_square_small_mask.png");
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem* system = GameSystem::GetInstance();

	Camera::GetInstance()->Initialize(static_cast<float>(Environment::GetInstance()->GetWindowSize().width), static_cast<float>(Environment::GetInstance()->GetWindowSize().height));
	
	system->Initialize();


		

	LoadDatas();


	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/

	SceneManager::GetInstance()->Initialize();

	// ウィンドウのxボタンが押されるまでループ.
	while (system->ProcessMessage()) {
		if (system->BeginFrame()) {
//
			/*=============================================================
			以下にゲームの更新処理を記述.
			=============================================================*/

			GlobalVariables::GetInstance()->Update();

			SceneManager::GetInstance()->Update();

			/*=============================================================
			以下にゲームの描画処理を記述.
			=============================================================*/
			system->DrawSetup();

			SceneManager::GetInstance()->Draw();

			system->EndFrame();
		}
	}

	system->Finalize();

	return 0;
}
