#pragma once
#include <memory>
#include "IScene.h"
#include "GameScene.h"

class SceneManager{
public:
	~SceneManager();

	static SceneManager* GetInstance();

	void Initialize();

	void Update();

	void Draw();

	void ChangeSceneUpdate();

	void ChengeScene(SceneName name);
private:
	GameScene* currentScene_ = new GameScene();

	SceneName sceneName_;

	bool isSceneChange_;
};

