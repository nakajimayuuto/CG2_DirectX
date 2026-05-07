#include "./Engine/Renderer/Camera.h"
#include "./Engine/SystemFile/GameSystem.h"
#include "./Managers/SceneManager.h"


// オーディオ類.

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem* system = GameSystem::GetInstance();

	system->Initialize();

	Camera::GetInstance()->Initialize(static_cast<float>(Environment::GetInstance()->GetWindowSize().width), static_cast<float>(Environment::GetInstance()->GetWindowSize().height));

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

			//ImGui::Begin("DirectionalLight");
			//
			//Vector3 dire = DirectionalLight::GetInstance()->GetDirectionalLightData()->direction;
			//
			//ImGui::SliderFloat3("direction", reinterpret_cast<float*>(&dire.x),-1.0f,1.0f);
			//
			//DirectionalLight::GetInstance()->GetDirectionalLightData()->direction = dire.Normalize();
			//
			//ImGui::End();

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
