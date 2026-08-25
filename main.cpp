#include "Satlib.h"
void LoadDatas();

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	GameSystem* system = GameSystem::GetInstance();

	Camera::GetInstance()->Initialize(static_cast<float>(Environment::GetInstance()->GetWindowSize().width), static_cast<float>(Environment::GetInstance()->GetWindowSize().height));

	system->Initialize();






	/*=============================================================
	ここから下がゲームの変数.
	=============================================================*/
	ParticleManager::GetInstance()->Initialize();
	LoadDatas();

	SceneManager::GetInstance()->Initialize();
	Environment::GetInstance()->SetWindowMode(WindowMode::kFullscreen);


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
			ImGui::DragFloat("intensity", &LightManager::GetInstance()->GetDirectionalLightData()->intensity,0.01f,0.0f,200.0f);

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
	ModelManager::GetInstance()->RegisterObj("celling", "Resource/Ground", "celling.obj");
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
	ModelManager::GetInstance()->RegisterObj("tutorial_wall", "Resource/wall", "tutorial_wall.obj");
	ModelManager::GetInstance()->RegisterObj("tutorial_celling", "Resource/wall", "tutorial_celling.obj");
	TextureManager::GetInstance()->RegisterTexture("bullet_bounce", "Resource/Bullet/bullet_bounce.png");
	TextureManager::GetInstance()->RegisterTexture("wall_soul", "Resource/wall/wall_soul.png");
	TextureManager::GetInstance()->RegisterTexture("door", "Resource/wall/door.png");
	TextureManager::GetInstance()->RegisterTexture("halberd_soul", "Resource/Halberd/halberd_soul.png");

	TextureManager::GetInstance()->RegisterTexture("effect_plane", "Resource/effects/effect_plane.png");
	TextureManager::GetInstance()->RegisterTexture("effect_fire", "Resource/effects/effect_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_big_fire", "Resource/effects/effect_big_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_blue_fire", "Resource/effects/effect_blue_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_big_blue_fire", "Resource/effects/effect_big_blue_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_slash", "Resource/effects/effect_slash.png");
	TextureManager::GetInstance()->RegisterTexture("effect_cross", "Resource/effects/effect_cross.png");
	TextureManager::GetInstance()->RegisterTexture("effect_star", "Resource/effects/effect_star.png");
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
	ParticleManager::GetInstance()->CreateNewParticles("big_blue_fire", TextureManager::GetInstance()->GetTextureInfo("effect_big_blue_fire"), BillboardType::kAllAxis, Particles::Move::kFire);
	ParticleManager::GetInstance()->SetParticleSize("big_blue_fire", { 0.5f,0.5f,0.5f });
	ParticleManager::GetInstance()->CreateNewParticles("star", TextureManager::GetInstance()->GetTextureInfo("effect_star"), BillboardType::kAllAxis, Particles::Move::kFire);
	ParticleManager::GetInstance()->SetParticleSize("star", { 0.2f,0.2f,0.2f });
	ParticleManager::GetInstance()->CreateNewParticles("death_cross", TextureManager::GetInstance()->GetTextureInfo("effect_cross"), BillboardType::kAllAxis, Particles::Move::kExplode);
	ParticleManager::GetInstance()->SetParticleSize("death_cross", { 0.2f,0.2f,0.2f });
	ParticleManager::GetInstance()->CreateNewParticles("explode", TextureManager::GetInstance()->GetTextureInfo("effect_cross"), BillboardType::kAllAxis, Particles::Move::kExplodeMonochrome);
	ParticleManager::GetInstance()->SetParticleSize("explode", { 0.2f,0.2f,0.2f });
	ParticleManager::GetInstance()->CreateNewParticles("charge", TextureManager::GetInstance()->GetTextureInfo("effect_cross"), BillboardType::kAllAxis, Particles::Move::kCharge);
	ParticleManager::GetInstance()->SetParticleSize("charge", { 0.2f,0.2f,0.2f });


	TextureManager::GetInstance()->RegisterTexture("press_a", "Resource/UI/press_a.png");
	TextureManager::GetInstance()->RegisterTexture("menu_start", "Resource/UI/menu_start.png");
	TextureManager::GetInstance()->RegisterTexture("menu_setting", "Resource/UI/menu_setting.png");
	TextureManager::GetInstance()->RegisterTexture("menu_return", "Resource/UI/menu_return.png");

	TextureManager::GetInstance()->RegisterTexture("ui_difficulty", "Resource/UI/difficulty.png");
	TextureManager::GetInstance()->RegisterTexture("difficulty_easy", "Resource/UI/difficulty_easy.png");
	TextureManager::GetInstance()->RegisterTexture("difficulty_normal", "Resource/UI/difficulty_normal.png");
	TextureManager::GetInstance()->RegisterTexture("difficulty_hard", "Resource/UI/difficulty_hard.png");
	TextureManager::GetInstance()->RegisterTexture("difficulty_normal_outline", "Resource/UI/difficulty_normal_outline.png");
	TextureManager::GetInstance()->RegisterTexture("difficulty_hard_outline", "Resource/UI/difficulty_hard_outline.png");
	TextureManager::GetInstance()->RegisterTexture("ui_return", "Resource/UI/return.png");
	TextureManager::GetInstance()->RegisterTexture("play_guid", "Resource/UI/play_guid.png");
	TextureManager::GetInstance()->RegisterTexture("gameover", "Resource/UI/gameover.png");
	TextureManager::GetInstance()->RegisterTexture("gameclear", "Resource/UI/gameclear.png");

	TextureManager::GetInstance()->RegisterTexture("pause", "Resource/UI/pause.png");
	TextureManager::GetInstance()->RegisterTexture("return_to_game", "Resource/UI/pause_return_to_game.png");
	TextureManager::GetInstance()->RegisterTexture("retry", "Resource/UI/pause_retry.png");
	TextureManager::GetInstance()->RegisterTexture("return_to_tutorial", "Resource/UI/pause_return_to_tutorial.png");
	TextureManager::GetInstance()->RegisterTexture("retry_phase2", "Resource/UI/pause_retry_phase2.png");
	TextureManager::GetInstance()->RegisterTexture("retry_last_jarona", "Resource/UI/pause_retry_last_jarona.png");
	TextureManager::GetInstance()->RegisterTexture("return_to_title", "Resource/UI/pause_return_to_title.png");

	TextureManager::GetInstance()->RegisterTexture("attack_to_x", "Resource/UI/attack_to_x.png");
	TextureManager::GetInstance()->RegisterTexture("dash_to_x", "Resource/UI/dash_to_x.png");
	TextureManager::GetInstance()->RegisterTexture("jump_to_a", "Resource/UI/jump_to_a.png");
	TextureManager::GetInstance()->RegisterTexture("move_to_l", "Resource/UI/move_to_l.png");
	TextureManager::GetInstance()->RegisterTexture("play_guid_tutorial_attack", "Resource/UI/play_guid_tutorial_attack.png");
	TextureManager::GetInstance()->RegisterTexture("play_guid_tutorial_dash", "Resource/UI/play_guid_tutorial_dash.png");

	TextureManager::GetInstance()->RegisterTexture("name_easy", "Resource/UI/boss_name_easy.png");
	TextureManager::GetInstance()->RegisterTexture("name_easy_mirror", "Resource/UI/boss_name_easy_mirror.png");
	TextureManager::GetInstance()->RegisterTexture("name_normal", "Resource/UI/boss_name_normal.png");
	TextureManager::GetInstance()->RegisterTexture("name_normal_mirror", "Resource/UI/boss_name_normal_mirror.png");
	TextureManager::GetInstance()->RegisterTexture("name_hard", "Resource/UI/boss_name_hard.png");
	TextureManager::GetInstance()->RegisterTexture("name_hard_mirror", "Resource/UI/boss_name_hard_mirror.png");
	TextureManager::GetInstance()->RegisterTexture("name_left_halberd", "Resource/UI/left_halberd_name.png");
	TextureManager::GetInstance()->RegisterTexture("name_right_halberd", "Resource/UI/right_halberd_name.png");

	SoundManager::GetInstance()->RegisterSound("mus_phase1_intro", "Resource/Sound/mus_phase1_intro.mp3");
	SoundManager::GetInstance()->RegisterSound("mus_phase1", "Resource/Sound/mus_phase1.mp3");
	SoundManager::GetInstance()->RegisterSound("mus_phase2_intro", "Resource/Sound/mus_phase2_intro.mp3");
	SoundManager::GetInstance()->RegisterSound("mus_phase2", "Resource/Sound/mus_phase2.mp3");

	SoundManager::GetInstance()->RegisterSound("test", "Resource/free_k.wav");
	SoundManager::GetInstance()->RegisterSound("snd_select", "Resource/Sound/snd_select.mp3");

}