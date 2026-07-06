#include "SceneManager.h"
#include "../Scene/GameScene.h"
#include "../Scene/TitleScene.h"
#include "InputManager.h"
#include "../Environment.h"
#include "../Engine/SystemFile/Debug.h"

SceneManager::~SceneManager() {
	delete currentScene_;
}

SceneManager* SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
};

void SceneManager::Initialize() {
	//currentScene_ = new TitleScene();

//#ifdef _DEBUG
	Debug::GetInstance()->LoadDebugSettings();
	currentScene_ = new GameScene();
//#endif // _DEBUG

	currentScene_->Initialize();
}

void SceneManager::Update() {
	if (InputManager::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		Environment::GetInstance()->GameFinished();
	}

	ChangeSceneUpdate();

	currentScene_->Update();
}

void SceneManager::Draw() {
	currentScene_->Draw();
}

void SceneManager::ChangeSceneUpdate() {
	if (!isSceneChange_) {
		return;
	}

	delete currentScene_;

	switch (sceneName_) {
	case SceneName::kGameScene:

		currentScene_ = new GameScene();
		currentScene_->Initialize();
		break;
	case SceneName::kTitleScene:
		currentScene_ = new TitleScene();
		currentScene_->Initialize();
		break;
	}

	isSceneChange_ = false;
}

void SceneManager::ChengeScene(SceneName name) {
	sceneName_ = name;
	isSceneChange_ = true;
}

void SceneManager::ReloadScene(){
	isSceneChange_ = true;
}
