#include "SceneManager.h"
#include "GameScene.h"
SceneManager::~SceneManager(){
	//currentScene_.release();
}

SceneManager* SceneManager::GetInstance() {
	static SceneManager instance;
	return &instance;
};

void SceneManager::Initialize() {
	currentScene_->Initialize();
}

void SceneManager::Update() {
	ChengeSceneUpdate();

	currentScene_->Update();
}

void SceneManager::Draw() {
	currentScene_->Draw();
}

void SceneManager::ChengeSceneUpdate(){
	if (!isSceneChange_) {
		return;
	}

	switch (sceneName_) {
	case SceneName::kGameScene:
		currentScene_ = new GameScene();
		currentScene_->Initialize();
		break;
	}

	isSceneChange_ = false;
}

void SceneManager::ChengeScene(SceneName name){
	sceneName_ = name;
	isSceneChange_ = true;
}
