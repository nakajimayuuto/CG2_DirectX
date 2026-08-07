#include "Satlib.h"
void LoadDatas();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem* system = GameSystem::GetInstance();

	Camera::GetInstance()->Initialize(static_cast<float>(Environment::GetInstance()->GetWindowSize().width), static_cast<float>(Environment::GetInstance()->GetWindowSize().height));

	system->Initialize();





	LoadDatas();

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

void LoadDatas() {
	TextureManager::GetInstance()->RegisterTexture("uvChecker", "Resource/uvChecker.png");
	TextureManager::GetInstance()->RegisterTexture("reticle", "Resource/reticle.png");
	TextureManager::GetInstance()->RegisterTexture("effect_plane", "Resource/EffectPlane/effect_plane.png");
	ModelManager::GetInstance()->RegisterObj("skydome", "Resource/skydome", "skydome.obj");
	ModelManager::GetInstance()->RegisterObj("creeking", "Resource/creeking", "creeking.obj");
	ModelManager::GetInstance()->RegisterObj("boss", "Resource/boss", "boss_ghost.obj", false);
	ModelManager::GetInstance()->RegisterObj("ground", "Resource/Ground", "ground.obj");
	ModelManager::GetInstance()->RegisterObj("player", "Resource/player_hovering_mode", "player.obj");
	ModelManager::GetInstance()->RegisterObj("drill_ghost", "Resource/drill_ghost", "drill_ghost.obj", false);
	ModelManager::GetInstance()->RegisterObj("halberd", "Resource/halberd", "halberd.obj", false);
	ModelManager::GetInstance()->RegisterObj("player_right_arm", "Resource/player_hovering_mode/right_arm", "right_arm.obj");
	ModelManager::GetInstance()->RegisterObj("player_left_arm", "Resource/player_hovering_mode/left_arm", "left_arm.obj");
	ModelManager::GetInstance()->RegisterObj("player_head", "Resource/player_hovering_mode/head", "head.obj");
	ModelManager::GetInstance()->RegisterObj("hammer_of_justice", "Resource/Hammer", "hammer_of_justice_uv.obj");
	ModelManager::GetInstance()->RegisterObj("enemy", "Resource/enemy", "enemy.obj");

}