#include "SceneManager.h"
#include "../Scene/GameScene.h"
#include "../Scene/TitleScene.h"
#include "InputManager.h"
#include "ImGuiManager.h"
#include "SoundManager.h"
#include "ParticleManager.h"
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



	// 停止中 / 一時停止中はゲームの更新処理を行わない(描画は行う).
	if (!ImGuiManager::GetInstance()->IsPlaying()) {
		if (ImGuiManager::GetInstance()->GetPlayState() == ImGuiManager::PlayState::kStop) {
			// 再生前でもカメラの行列は作っておく(一時停止中は止めた時の行列をそのまま使う).
			Camera::GetInstance()->Update();
		}
		return;
	}

	ChangeSceneUpdate();

	currentScene_->Update();

	ParticleManager::GetInstance()->Update();
}

void SceneManager::Draw() {
	currentScene_->Draw();
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
}

void SceneManager::PlayControlUpdate() {
	ImGuiManager* imGuiManager = ImGuiManager::GetInstance();

	// 終了ボタン → 再生前の状態に戻す.
	// (ボタンは前フレームの描画中に押されるので、シーンの破棄はここ(次フレームの更新の最初)で行う).
	if (imGuiManager->ConsumeStopRequested()) {
		ResetToPlayStartState();
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
	CreateScene(sceneName_);
}
