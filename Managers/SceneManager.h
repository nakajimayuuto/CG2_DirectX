#pragma once
#include <memory>
#include "../Scene/IScene.h"

class GameScene;

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
	IScene* currentScene_ = nullptr;

	SceneName sceneName_;

	bool isSceneChange_;
};

