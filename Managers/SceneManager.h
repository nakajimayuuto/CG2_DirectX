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

	void ReloadScene();
private:
	std::unique_ptr<IScene> currentScene_ = nullptr;

	SceneName sceneName_;

	bool isSceneChange_;
};

