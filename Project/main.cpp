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
	Sprite sprite;
	sprite.Initialize(TextureManager::GetInstance()->GetTextureInfo("bullet_bounce"));
#ifndef _DEBUG
	Environment::GetInstance()->SetWindowMode(WindowMode::kFullscreen);
#endif // _DEBUG

	Vector3 color_;
	color_ = { 1.0f,1.0f,1.0f };
	float r = 0.0f;
	float num_ = 1024.0f;
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
			r = Degree(r);
			ImGui::DragFloat("spriteRotate", &r, 1.0f, -360.0f, 360.0f);
			Vector2 anchor = sprite.GetAncor();
			ImGui::DragFloat2("spriteAncor", reinterpret_cast<float*>(&anchor), 0.01f, -1.0f, 2.0f);
			sprite.SetAncor(anchor);
			r = Radian(r);
			ImGui::DragFloat("intensity", &LightManager::GetInstance()->GetDirectionalLightData()->intensity, 0.01f, 0.0f, 200.0f);
			ImGui::DragFloat("num", &num_, 0.01f, 0.0f, 10000.0f);
			ImGui::ColorEdit3("parColor", reinterpret_cast<float*>(&color_));

			if (ImGui::Button("numcreate")) {
				ParticleManager::GetInstance()->SpawnNumbers(num_, Transform::GetInitialValue(), color_);
			}

			ImGui::End();
#endif // _DEBUG
			/*=============================================================
			以下にゲームの描画処理を記述.
			=============================================================*/
			//system->DrawSetup();

			sprite.Draw(Transform2D::GetTransformValue({ 1.0f,1.0f }, r, {0.0f,0.0f}));

			SceneManager::GetInstance()->Draw();

			system->EndFrame();
		}
	}

	system->Finalize();

	return 0;
}

void LoadDatas() {
	//ModelManager::GetInstance()->RegisterObj("boss", "Resource/boss", "boss_ghost.obj", false);
	ModelManager::GetInstance()->RegisterObj("ground", "Resources/Ground", "ground.obj");
	ModelManager::GetInstance()->RegisterObj("celling", "Resources/Ground", "celling.obj");
	ModelManager::GetInstance()->RegisterObj("drill_ghost", "Resources/drill_ghost", "drill_ghost.obj", false);
	ModelManager::GetInstance()->RegisterObj("halberd", "Resources/halberd", "halberd.obj", false);
	ModelManager::GetInstance()->RegisterObj("wall", "Resources/wall", "wall.obj");
	ModelManager::GetInstance()->RegisterObj("bullet_crystal", "Resources/Bullet", "bullet_crystal.obj");
	ModelManager::GetInstance()->RegisterObj("spike", "Resources/Spike", "spike.obj");
	ModelManager::GetInstance()->RegisterObj("tutorial_wall", "Resources/wall", "tutorial_wall.obj");
	ModelManager::GetInstance()->RegisterObj("tutorial_celling", "Resources/wall", "tutorial_celling.obj");
	TextureManager::GetInstance()->RegisterTexture("bullet_bounce", "Resources/Bullet/bullet_bounce.png");
	TextureManager::GetInstance()->RegisterTexture("wall_soul", "Resources/wall/wall_soul.png");
	TextureManager::GetInstance()->RegisterTexture("door", "Resources/wall/door.png");
	//TextureManager::GetInstance()->RegisterTexture("halberd_soul", "Resource/Halberd/halberd_soul.png");

	TextureManager::GetInstance()->RegisterTexture("effect_plane", "Resources/effects/effect_plane.png");
	TextureManager::GetInstance()->RegisterTexture("effect_fire", "Resources/effects/effect_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_big_fire", "Resources/effects/effect_big_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_blue_fire", "Resources/effects/effect_blue_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_big_blue_fire", "Resources/effects/effect_big_blue_fire.png");
	TextureManager::GetInstance()->RegisterTexture("effect_slash", "Resources/effects/effect_slash.png");
	TextureManager::GetInstance()->RegisterTexture("effect_cross", "Resources/effects/effect_cross.png");
	TextureManager::GetInstance()->RegisterTexture("effect_star", "Resources/effects/effect_star.png");
	ParticleManager::GetInstance()->CreateNewParticles("normal", TextureManager::GetInstance()->GetTextureInfo("effect_plane"), BillboardType::kAllAxis, Particles::Move::kNormal);
	ParticleManager::GetInstance()->CreateNewParticles("fire", TextureManager::GetInstance()->GetTextureInfo("effect_fire"), BillboardType::kAllAxis, Particles::Move::kFire);
	ParticleManager::GetInstance()->SetParticleSize("fire", { 0.2f,0.2f,0.2f });
	ParticleManager::GetInstance()->CreateNewParticles("blue_fire", TextureManager::GetInstance()->GetTextureInfo("effect_blue_fire"), BillboardType::kAllAxis, Particles::Move::kFire);
	ParticleManager::GetInstance()->SetParticleSize("blue_fire", { 0.2f,0.2f,0.2f });
	ParticleManager::GetInstance()->CreateNewParticles("slash", TextureManager::GetInstance()->GetTextureInfo("effect_slash"), BillboardType::kAllAxis, Particles::Move::kSlash);
	ParticleManager::GetInstance()->SetParticleSize("slash", { 0.5f,0.5f,0.5f });
	ParticleManager::GetInstance()->CreateNewParticles("cross", TextureManager::GetInstance()->GetTextureInfo("effect_cross"), BillboardType::kAllAxis, Particles::Move::kSlash);
	ParticleManager::GetInstance()->SetParticleSize("cross", { 0.2f,0.2f,0.2f });
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


	TextureManager::GetInstance()->RegisterTexture("press_a", "Resources/UI/press_a.png");
	TextureManager::GetInstance()->RegisterTexture("menu_start", "Resources/UI/menu_start.png");
	TextureManager::GetInstance()->RegisterTexture("menu_setting", "Resources/UI/menu_setting.png");
	TextureManager::GetInstance()->RegisterTexture("menu_return", "Resources/UI/menu_return.png");

	TextureManager::GetInstance()->RegisterTexture("ui_difficulty", "Resources/UI/difficulty.png");
	TextureManager::GetInstance()->RegisterTexture("difficulty_easy", "Resources/UI/difficulty_easy.png");
	TextureManager::GetInstance()->RegisterTexture("difficulty_normal", "Resources/UI/difficulty_normal.png");
	TextureManager::GetInstance()->RegisterTexture("difficulty_hard", "Resources/UI/difficulty_hard.png");
	TextureManager::GetInstance()->RegisterTexture("difficulty_normal_outline", "Resources/UI/difficulty_normal_outline.png");
	TextureManager::GetInstance()->RegisterTexture("difficulty_hard_outline", "Resources/UI/difficulty_hard_outline.png");
	TextureManager::GetInstance()->RegisterTexture("ui_return", "Resources/UI/return.png");
	TextureManager::GetInstance()->RegisterTexture("play_guid", "Resources/UI/play_guid.png");
	TextureManager::GetInstance()->RegisterTexture("gameover", "Resources/UI/gameover.png");
	TextureManager::GetInstance()->RegisterTexture("gameclear", "Resources/UI/gameclear.png");

	TextureManager::GetInstance()->RegisterTexture("pause", "Resources/UI/pause.png");
	TextureManager::GetInstance()->RegisterTexture("return_to_game", "Resources/UI/pause_return_to_game.png");
	TextureManager::GetInstance()->RegisterTexture("retry", "Resources/UI/pause_retry.png");
	TextureManager::GetInstance()->RegisterTexture("return_to_tutorial", "Resources/UI/pause_return_to_tutorial.png");
	TextureManager::GetInstance()->RegisterTexture("retry_phase2", "Resources/UI/pause_retry_phase2.png");
	TextureManager::GetInstance()->RegisterTexture("retry_last_jarona", "Resources/UI/pause_retry_last_jarona.png");
	TextureManager::GetInstance()->RegisterTexture("return_to_title", "Resources/UI/pause_return_to_title.png");

	TextureManager::GetInstance()->RegisterTexture("attack_to_x", "Resources/UI/attack_to_x.png");
	TextureManager::GetInstance()->RegisterTexture("dash_to_x", "Resources/UI/dash_to_x.png");
	TextureManager::GetInstance()->RegisterTexture("jump_to_a", "Resources/UI/jump_to_a.png");
	TextureManager::GetInstance()->RegisterTexture("move_to_l", "Resources/UI/move_to_l.png");
	TextureManager::GetInstance()->RegisterTexture("play_guid_tutorial_attack", "Resources/UI/play_guid_tutorial_attack.png");
	TextureManager::GetInstance()->RegisterTexture("play_guid_tutorial_dash", "Resources/UI/play_guid_tutorial_dash.png");
	TextureManager::GetInstance()->RegisterTexture("tutorial_attack_info", "Resources/UI/tutorial_attack_info.png");
	TextureManager::GetInstance()->RegisterTexture("tutorial_damage_info", "Resources/UI/tutorial_damage_info.png");
	TextureManager::GetInstance()->RegisterTexture("skip_to_lr", "Resources/UI/skip_to_lr.png");
	TextureManager::GetInstance()->RegisterTexture("pause_to_hamburger", "Resources/UI/pause_to_hamburger.png");

	TextureManager::GetInstance()->RegisterTexture("name_easy", "Resources/UI/boss_name_easy.png");
	TextureManager::GetInstance()->RegisterTexture("name_easy_mirror", "Resources/UI/boss_name_easy_mirror.png");
	TextureManager::GetInstance()->RegisterTexture("name_normal", "Resources/UI/boss_name_normal.png");
	TextureManager::GetInstance()->RegisterTexture("name_normal_mirror", "Resources/UI/boss_name_normal_mirror.png");
	TextureManager::GetInstance()->RegisterTexture("name_hard", "Resources/UI/boss_name_hard.png");
	TextureManager::GetInstance()->RegisterTexture("name_hard_mirror", "Resources/UI/boss_name_hard_mirror.png");
	TextureManager::GetInstance()->RegisterTexture("name_left_halberd", "Resources/UI/left_halberd_name.png");
	TextureManager::GetInstance()->RegisterTexture("name_right_halberd", "Resources/UI/right_halberd_name.png");

	TextureManager::GetInstance()->RegisterTexture("end_to_hamburger", "Resources/UI/end_to_hamburger.png");
	TextureManager::GetInstance()->RegisterTexture("title", "Resources/UI/title.png");

	ParticleManager::GetInstance()->CreateNewParticles("normal", TextureManager::GetInstance()->GetTextureInfo("effect_plane"), BillboardType::kAllAxis, Particles::Move::kNormal);
	ParticleManager::GetInstance()->CreateNewParticles("fire", TextureManager::GetInstance()->GetTextureInfo("effect_fire"), BillboardType::kAllAxis, Particles::Move::kFire);

	SoundManager::GetInstance()->RegisterSound("mus_phase1_intro", "Resources/Sound/mus_phase1_intro.wav");
	SoundManager::GetInstance()->RegisterSound("mus_phase1", "Resources/Sound/mus_phase1.wav");
	SoundManager::GetInstance()->RegisterSound("mus_phase2_intro", "Resources/Sound/mus_phase2_intro.wav");
	SoundManager::GetInstance()->RegisterSound("mus_phase2", "Resources/Sound/mus_phase2.wav");
	SoundManager::GetInstance()->RegisterSound("mus_title", "Resources/Sound/mus_title.mp3");

	SoundManager::GetInstance()->RegisterSound("test", "Resources/free_k.wav");
	SoundManager::GetInstance()->RegisterSound("snd_select", "Resources/Sound/snd_select.mp3");

	SoundManager::GetInstance()->RegisterSound("snd_near_attack", "Resources/Sound/snd_attack_near.mp3");
	SoundManager::GetInstance()->RegisterSound("snd_near_attack_third", "Resources/Sound/snd_attack_near_third.wav");
	SoundManager::GetInstance()->RegisterSound("snd_bullet_shot", "Resources/Sound/snd_bullet_shot.mp3");
	SoundManager::GetInstance()->RegisterSound("snd_wave_shot", "Resources/Sound/snd_wave_shot.mp3");
	SoundManager::GetInstance()->RegisterSound("snd_parry", "Resources/Sound/snd_parry.mp3");
	SoundManager::GetInstance()->RegisterSound("snd_attack_wind", "Resources/Sound/snd_attack_wind.wav");
	SoundManager::GetInstance()->RegisterSound("snd_shine", "Resources/Sound/snd_shine.wav");
	SoundManager::GetInstance()->RegisterSound("snd_explode", "Resources/Sound/snd_explode.wav");
	SoundManager::GetInstance()->RegisterSound("snd_explode_mini", "Resources/Sound/snd_explode_mini.wav");
	SoundManager::GetInstance()->RegisterSound("snd_boss_damage", "Resources/Sound/snd_boss_damage.mp3");
	SoundManager::GetInstance()->RegisterSound("snd_player_damage", "Resources/Sound/snd_player_damage.mp3");
	SoundManager::GetInstance()->RegisterSound("snd_special_attack", "Resources/Sound/snd_special_attack.mp3");
	SoundManager::GetInstance()->RegisterSound("snd_step", "Resources/Sound/snd_step.wav");
	SoundManager::GetInstance()->RegisterSound("snd_drill", "Resources/Sound/snd_drill.mp3");
	SoundManager::GetInstance()->RegisterSound("snd_parry", "Resources/Sound/snd_parry.mp3");

}