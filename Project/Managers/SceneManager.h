#pragma once
#include <memory>
#include "../Scene/IScene.h"
#include "DifficultyManager.h"

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
	/// <summary>
	/// 指定したシーンを生成して初期化する.
	/// </summary>
	void CreateScene(SceneName name);

	/// <summary>
	/// ImGuiのシーン切り替えGUIから指定されたシーンを読み込む.
	/// </summary>
	void LoadSceneFromEditor(SceneName name);

	/// <summary>
	/// ImGuiManagerの再生 / 一時停止 / 終了 ボタンの結果を反映する.
	/// </summary>
	void PlayControlUpdate();

	/// <summary>
	/// 再生ボタンが押される前の状態に戻す(終了ボタン用).
	/// </summary>
	void ResetToPlayStartState();
private:
	std::unique_ptr<IScene> currentScene_ = nullptr;

	SceneName sceneName_;

	bool isSceneChange_;

	// 再生ボタンが押された時の状態(終了ボタンでここに戻す).
	SceneName playStartSceneName_ = SceneName::kTitleScene;
	Difficulty playStartDifficulty_ = Difficulty::kDifficultyEasy;
};
