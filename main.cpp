#include "Satlib.h"
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem* system = GameSystem::GetInstance();

	Camera::GetInstance()->Initialize(static_cast<float>(Environment::GetInstance()->GetWindowSize().width), static_cast<float>(Environment::GetInstance()->GetWindowSize().height));

	system->Initialize();




	TextureManager::GetInstance()->RegisterTexture("window_back", "Resource/Window/window_back.png");
	TextureManager::GetInstance()->RegisterTexture("window_mask", "Resource/Window/window_mask.png");
	TextureManager::GetInstance()->RegisterTexture("window_s_l_back", "Resource/Window/window_square_large_back.png");
	TextureManager::GetInstance()->RegisterTexture("window_s_l_mask", "Resource/Window/window_square_large_mask.png");
	TextureManager::GetInstance()->RegisterTexture("window_s_s_mask", "Resource/Window/window_square_small_back.png");
	TextureManager::GetInstance()->RegisterTexture("window_s_s_back", "Resource/Window/window_square_small_mask.png");

	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	ModelManager::GetInstance()->RegisterObj("plane", "Resource/Evaluation", "plane.obj");
	ModelManager::GetInstance()->RegisterObj("teapot", "Resource/Evaluation", "teapot.obj");
	ModelManager::GetInstance()->RegisterObj("bunny", "Resource/Evaluation", "bunny.obj");
	ModelManager::GetInstance()->RegisterObj("multiMesh", "Resource/Evaluation", "multiMesh.obj");
	ModelManager::GetInstance()->RegisterObj("multiMaterial", "Resource/Evaluation", "multiMaterial.obj");
	ModelManager::GetInstance()->RegisterObj("suzanne", "Resource/Evaluation", "suzanne.obj");



	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/
	SoundManager::GetInstance()->RegisterSound("test", "Resource/free_k.wav");

	SceneManager::GetInstance()->Initialize();


	LightManager::GetInstance()->GetDirectionalLightData()->intensity = 0.5f;
	// ウィンドウのxボタンが押されるまでループ.
	while (system->ProcessMessage()) {
		if (system->BeginFrame()) {
			/*=============================================================
			以下にゲームの更新処理を記述.
			=============================================================*/

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
