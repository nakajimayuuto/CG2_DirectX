#include "Satlib.h"
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem* system = GameSystem::GetInstance();

	Camera::GetInstance()->Initialize(static_cast<float>(Environment::GetInstance()->GetWindowSize().width), static_cast<float>(Environment::GetInstance()->GetWindowSize().height));

	system->Initialize();







	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/

	SceneManager::GetInstance()->Initialize();

	SoundManager::GetInstance()->RegisterSound("test", "Resource/free_k.wav");
	SoundData data = SoundManager::GetInstance()->GetSoundData("test");
	//SoundManager::GetInstance()->SoundPlay(data,1.0f, 1.0f, kBGM, true, "test");


	// ウィンドウのxボタンが押されるまでループ.
	while (system->ProcessMessage()) {
		if (system->BeginFrame()) {
			//
						/*=============================================================
						以下にゲームの更新処理を記述.
						=============================================================*/

			GlobalVariables::GetInstance()->Update();

			SceneManager::GetInstance()->Update();

			ImGui::Begin("bgmTest");
			if (ImGui::Button("start")) {
				SoundManager::GetInstance()->SoundPlay(data, 1.0f, 1.0f, kBGM, true, "test");
			}
			if (ImGui::Button("stop")) {
				SoundManager::GetInstance()->SoundStop("test");
			}
			if (ImGui::Button("noLoopStart")) {
				SoundManager::GetInstance()->SoundPlay(data, 1.0f, 1.0f, kBGM, false, "noLtest");
			}
			if (ImGui::Button("se")) {
				SoundManager::GetInstance()->SoundPlay(data, 1.0f, 1.0f, kSoundEffect);
			}
			ImGui::End();

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
