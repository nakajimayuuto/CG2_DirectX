#include "Framework.h"
#include "Satlib.h"
void Framework::Initialize() {

	Camera::GetInstance()->Initialize(static_cast<float>(Environment::GetInstance()->GetWindowSize().width), static_cast<float>(Environment::GetInstance()->GetWindowSize().height));

	GameSystem::GetInstance()->Initialize();

	ParticleManager::GetInstance()->Initialize();

}
void Framework::Update() {
	GlobalVariables::GetInstance()->Update();

}
void Framework::Draw() {
	GameSystem::GetInstance()->EndFrame();

}

void Framework::Finalize() {
	GameSystem::GetInstance()->Finalize();

}

void Framework::Run(){
	Initialize();
	GameSystem* system = GameSystem::GetInstance();
	// ウィンドウのxボタンが押されるまでループ.
	while (system->ProcessMessage()) {
		if (system->BeginFrame()) {
			/*=============================================================
			以下にゲームの更新処理を記述.
			=============================================================*/

			Update();
			/*=============================================================
			以下にゲームの描画処理を記述.
			=============================================================*/
			Draw();
		}
	}

	Finalize();
}
