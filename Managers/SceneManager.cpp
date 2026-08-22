#include "SceneManager.h"
#include "../Scene/GameScene.h"
#include "../Scene/TitleScene.h"
#include "InputManager.h"
#include "../Environment.h"
#include "../Engine/SystemFile/Debug.h"
#include "CollisionManager.h"

SceneManager::~SceneManager() {
}

SceneManager* SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
};

void SceneManager::Initialize() {
	CollisionManager::GetInstance()->CollisionAttributeInitialize();
	currentScene_ = std::make_unique<TitleScene>();
	sceneName_ = SceneName::kTitleScene;
	#ifdef _DEBUG
	Debug::GetInstance()->LoadDebugSettings();
	currentScene_ = std::make_unique<GameScene>();
	sceneName_ = SceneName::kGameScene;
	gGamePhase = GamePhase::kBossPhase1;
	#endif // _DEBUG

	currentScene_->Initialize();
}

void SceneManager::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		Environment::GetInstance()->GameFinished();
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

	switch (sceneName_) {
	case SceneName::kGameScene:

		currentScene_ = std::make_unique <GameScene>();
		currentScene_->Initialize();
		break;
	case SceneName::kTitleScene:
		currentScene_ = std::make_unique <TitleScene>();
		currentScene_->Initialize();
		break;
	}

	isSceneChange_ = false;
}

void SceneManager::ChengeScene(SceneName name) {
	sceneName_ = name;
	isSceneChange_ = true;
}

void SceneManager::ReloadScene() {
	isSceneChange_ = true;
}
