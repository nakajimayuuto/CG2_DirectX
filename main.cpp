#include "Satlib.h"
void LoadDatas();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem* system = GameSystem::GetInstance();

	Camera::GetInstance()->Initialize(static_cast<float>(Environment::GetInstance()->GetWindowSize().width), static_cast<float>(Environment::GetInstance()->GetWindowSize().height));

	system->Initialize();






	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/
	SoundManager::GetInstance()->RegisterSound("test", "Resource/free_k.wav");
	ParticleManager::GetInstance()->Initialize();
	LoadDatas();




	SceneManager::GetInstance()->Initialize();
	LightManager::GetInstance()->GetDirectionalLightData()->intensity = 0.1f;
	LightManager::GetInstance()->GetDirectionalLightData()->color = {1.0f,0.5f,0.5f,1.0f};
	// ウィンドウのxボタンが押されるまでループ.
	while (system->ProcessMessage()) {
		if (system->BeginFrame()) {
			/*=============================================================
			以下にゲームの更新処理を記述.
			=============================================================*/

			GlobalVariables::GetInstance()->Update();

			SceneManager::GetInstance()->Update();
#ifdef _DEBUG
			ImGui::Begin("aa");
			ImGui::DragFloat("intensity", &LightManager::GetInstance()->GetDirectionalLightData()->intensity,0.01f,0.0f,1.0f);

			ImGui::End();
#endif // _DEBUG

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
	ModelManager::GetInstance()->RegisterObj("wall", "Resource/wall", "wall.obj");
	ModelManager::GetInstance()->RegisterObj("bullet_crystal", "Resource/Bullet", "bullet_crystal.obj");
	ModelManager::GetInstance()->RegisterObj("spike", "Resource/Spike", "spike.obj");
	ModelManager::GetInstance()->RegisterObj("tutorial_wall", "Resource/Wall", "tutorial_wall.obj");
	TextureManager::GetInstance()->RegisterTexture("bullet_bounce", "Resource/Bullet/bullet_bounce.png");

	TextureManager::GetInstance()->RegisterTexture("effect_plane", "Resource/effects/effect_plane.png");
	TextureManager::GetInstance()->RegisterTexture("effect_fire", "Resource/effects/effect_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_big_fire", "Resource/effects/effect_big_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_blue_fire", "Resource/effects/effect_blue_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_slash", "Resource/effects/effect_slash.png");
	TextureManager::GetInstance()->RegisterTexture("effect_cross", "Resource/effects/effect_cross.png");
	ParticleManager::GetInstance()->CreateNewParticles("normal",TextureManager::GetInstance()->GetTextureInfo("effect_plane"), BillboardType::kAllAxis,Particles::Move::kNormal);
	ParticleManager::GetInstance()->CreateNewParticles("fire",TextureManager::GetInstance()->GetTextureInfo("effect_fire"), BillboardType::kAllAxis,Particles::Move::kFire);
	ParticleManager::GetInstance()->SetParticleSize("fire", {0.2f,0.2f,0.2f});
	ParticleManager::GetInstance()->CreateNewParticles("blue_fire",TextureManager::GetInstance()->GetTextureInfo("effect_blue_fire"), BillboardType::kAllAxis,Particles::Move::kFire);
	ParticleManager::GetInstance()->SetParticleSize("blue_fire", {0.2f,0.2f,0.2f});
	ParticleManager::GetInstance()->CreateNewParticles("slash",TextureManager::GetInstance()->GetTextureInfo("effect_slash"), BillboardType::kAllAxis,Particles::Move::kSlash);
	ParticleManager::GetInstance()->SetParticleSize("slash", {0.5f,0.5f,0.5f});
	ParticleManager::GetInstance()->CreateNewParticles("cross",TextureManager::GetInstance()->GetTextureInfo("effect_cross"), BillboardType::kAllAxis,Particles::Move::kSlash);
	ParticleManager::GetInstance()->SetParticleSize("cross", {0.2f,0.2f,0.2f});
	ParticleManager::GetInstance()->CreateNewParticles("big_fire", TextureManager::GetInstance()->GetTextureInfo("effect_big_fire"), BillboardType::kAllAxis, Particles::Move::kFire);
	ParticleManager::GetInstance()->SetParticleSize("big_fire", { 0.5f,0.5f,0.5f });

}