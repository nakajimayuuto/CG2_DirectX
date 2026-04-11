#include "Camera.h"
#include "GameSystem.h"
#include "GameScene.h"


// オーディオ類.
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <fstream>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem* system = GameSystem::GetInstance();

	system->Initialize();

	Camera::GetInstance()->Initialize(system->GetWindowSize().width, system->GetWindowSize().height);

	/*=============================================================
	オーディオ類(07_00).
	=============================================================*/
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;




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
