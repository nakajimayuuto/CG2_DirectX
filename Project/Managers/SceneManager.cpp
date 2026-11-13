#include "SceneManager.h"
#include "../Scene/GameScene.h"
#include "../Scene/TitleScene.h"
#include "InputManager.h"
#include "ImGuiManager.h"
#include "SoundManager.h"
#include "ParticleManager.h"
#include "EmitterManager.h"
#include "ParticleSettingsManager.h"
#include "nameof.hpp"
#include "../Engine/Environment/Environment.h"
#include "../Engine/SystemFile/Debug.h"
#include "../Engine/SystemFile/DeltaTime.h"
#include "../Engine/Renderer/Camera.h"
#include "CollisionManager.h"

SceneManager::~SceneManager() {
}

SceneManager* SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
};

void SceneManager::Initialize() {
	CollisionManager::GetInstance()->CollisionAttributeInitialize();

	// 停止状態で始まる場合、シーンの初期化で鳴らすBGMが流れないよう先に音を止めておく.
	SoundManager::GetInstance()->SetPaused(!ImGuiManager::GetInstance()->IsPlaying());

	sceneName_ = SceneName::kTitleScene;
	isSceneChange_ = false;
	#ifdef _DEBUG
	//Debug::GetInstance()->LoadDebugSettings();
	//sceneName_ = SceneName::kGameScene;
	//gGamePhase = GamePhase::kBossPhase2;
	//DifficultyManager::GetInstance()->SetCurrentDifficulty(Difficulty::kDifficultyHard);
	#endif // _DEBUG

	CreateScene(sceneName_);

	playStartSceneName_ = sceneName_;
	playStartDifficulty_ = DifficultyManager::GetInstance()->GetCurrentDifficulty();
}

void SceneManager::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		Environment::GetInstance()->GameFinished();
	}

	if (InputManager::GetInstance()->TriggerKey(DIK_F11)) {
		if (Environment::GetInstance()->GetWindowMode() == WindowMode::kFullscreen) {
			Environment::GetInstance()->SetWindowMode(WindowMode::kWindowed);
		} else {
			Environment::GetInstance()->SetWindowMode(WindowMode::kFullscreen);
		}
	}

	PlayControlUpdate();

	// GUIで変えたパーティクルを反映する(描画コマンドを積む前に行う).
	ParticleSettingsManager::GetInstance()->ApplyChanges();



	// 停止中 / 一時停止中はゲームの更新処理を行わない(描画は行う).
	if (!ImGuiManager::GetInstance()->IsPlaying()) {
		Camera* camera = Camera::GetInstance();
		// 再生していない間もF3でデバッグカメラを切り替えられる(再生中は各シーンで切り替える).
		if (InputManager::GetInstance()->TriggerKey(DIK_F3)) {
			camera->ChangeCameraMode();
		}
		// 再生前でもカメラの行列は作っておく(一時停止中は止めた時の行列をそのまま使う).
		// デバッグカメラを使っている時は、停止中 / 一時停止中もカメラを動かせる.
		if (ImGuiManager::GetInstance()->GetPlayState() == ImGuiManager::PlayState::kStop || camera->GetUseDebugCamera()) {
			camera->Update();
		}
		return;
	}

	ChangeSceneUpdate();

	currentScene_->Update();

	// GUIで設置したエミッターがパーティクルを出す.
	EmitterManager::GetInstance()->Update();

	ParticleManager::GetInstance()->Update();
}

void SceneManager::Draw() {
	currentScene_->Draw();

	// GUIで設置したエミッターの範囲(Debug版のみ).
	EmitterManager::GetInstance()->Draw();
}

void SceneManager::ChangeSceneUpdate() {
	if (!isSceneChange_) {
		return;
	}

	CreateScene(sceneName_);

	isSceneChange_ = false;
}

void SceneManager::ChengeScene(SceneName name) {
	sceneName_ = name;
	isSceneChange_ = true;
}

void SceneManager::ReloadScene() {
	isSceneChange_ = true;
}

void SceneManager::CreateScene(SceneName name) {
	switch (name) {
	case SceneName::kGameScene:
		currentScene_ = std::make_unique<GameScene>();
		break;
	case SceneName::kTitleScene:
		currentScene_ = std::make_unique<TitleScene>();
		break;
	}

	currentScene_->Initialize();

	// GUIで作ったパーティクルを読み込む(最初の1回だけ. Resources/Particle/particles.json).
	ParticleSettingsManager::GetInstance()->Load();

	// このシーンにGUIで設置したエミッターを読み込む(Resources/Emitter/シーン名.json).
	EmitterManager::GetInstance()->LoadScene(std::string(NAMEOF_ENUM(name)));

	// シーン切り替えGUIに今のシーンを表示する.
	ImGuiManager::GetInstance()->SetCurrentSceneName(name);
}

void SceneManager::LoadSceneFromEditor(SceneName name) {
	// 前のシーンで鳴っていた音・出ていたパーティクルを片付ける.
	SoundManager::GetInstance()->StopAllSounds();
	ParticleManager::GetInstance()->ClearParticles();

	DeltaTime::GetInstance()->Initialize();
	Camera::GetInstance()->Initialize();

	sceneName_ = name;
	isSceneChange_ = false;
	CreateScene(sceneName_);

	// 停止中に読み込んだ場合は、このシーンが再生の開始シーンになる.
	// (再生中 / 一時停止中に読み込んだ場合は、終了ボタンで再生前のシーンに戻る).
	if (ImGuiManager::GetInstance()->GetPlayState() == ImGuiManager::PlayState::kStop) {
		playStartSceneName_ = sceneName_;
	}
}

void SceneManager::PlayControlUpdate() {
	ImGuiManager* imGuiManager = ImGuiManager::GetInstance();

	// 終了ボタン → 再生前の状態に戻す.
	// (ボタンは前フレームの描画中に押されるので、シーンの破棄はここ(次フレームの更新の最初)で行う).
	if (imGuiManager->ConsumeStopRequested()) {
		ResetToPlayStartState();
	}

	// シーン切り替えGUIの読み込みボタン → 選択したシーンを読み込む(停止中 / 一時停止中でも読み込む).
	SceneName loadSceneName;
	if (imGuiManager->ConsumeLoadSceneRequest(loadSceneName)) {
		LoadSceneFromEditor(loadSceneName);
	}

	// 停止状態から再生 → 終了ボタンで戻ってこられるよう今の状態を記録する.
	if (imGuiManager->ConsumePlayStarted()) {
		playStartSceneName_ = sceneName_;
		playStartDifficulty_ = DifficultyManager::GetInstance()->GetCurrentDifficulty();
		// 停止中に積まれていた時間を進めないよう、経過時間の計測をやり直す.
		DeltaTime::GetInstance()->Initialize();
	}

	// 再生中以外は音も止める(一時停止なら再開時に続きから鳴る).
	SoundManager::GetInstance()->SetPaused(!imGuiManager->IsPlaying());
}

void SceneManager::ResetToPlayStartState() {
	// 再生中に鳴っていた音・パーティクルを片付ける.
	SoundManager::GetInstance()->StopAllSounds();
	ParticleManager::GetInstance()->ClearParticles();

	// 時間・カメラ・難易度を再生前に戻す.
	DeltaTime::GetInstance()->Initialize();
	Camera::GetInstance()->Initialize();
	DifficultyManager::GetInstance()->SetCurrentDifficulty(playStartDifficulty_);

	// 再生ボタンを押した時のシーンを作り直す.
	sceneName_ = playStartSceneName_;
	isSceneChange_ = false;
	// (GUIで設置したエミッターは、再生中に置いた / 動かした内容もそのまま残る. EmitterManager::LoadScene).
	CreateScene(sceneName_);
}
