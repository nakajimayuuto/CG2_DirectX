#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "../Engine/Renderer/ParticleEmitterEntity.h"
#include "../externals/nlohmann/json.hpp"

/// <summary>
/// GUIで設置したパーティクルのエミッター(ParticleEmitterEntity)をシーンごとに管理する.
/// ・"エミッター"ウィンドウの追加ボタンで設置し、Inspectorの各エミッターのタブで調整・削除する.
/// ・保存すると Resources/Emitter/シーン名.json に書き出し、シーンを作った時に読み込む(Release版でも読み込んで動く).
/// ・停止中のカメラは再生中のカメラと位置が違うことがあるので、再生中 / 一時停止中に置いて調整できるよう、
///   終了ボタンやシーンの切り替えでシーンを作り直しても編集中の内容は残す(アプリを閉じるまで. 残すには保存する).
/// </summary>
class EmitterManager {
public:
	// 保存先のフォルダ.
	static inline const std::string kDirectoryPath = "Resources/Emitter/";

	static EmitterManager* GetInstance();

	/// <summary>
	/// シーンを作った時に呼ぶ. 今のエミッターを消して、そのシーンの保存ファイルから読み込む.
	/// </summary>
	void LoadScene(const std::string& sceneName);

	// 再生中に呼ぶ. 各エミッターがパーティクルを出す.
	void Update();

	// 各エミッターの範囲を描画する(Debug版のみ).
	void Draw();

	/// <summary>
	/// "エミッター"ウィンドウと各エミッターのInspectorのタブを描画する(ImGuiManager::EndFrameで呼ぶ). Release版では何もしない.
	/// </summary>
	void DrawGui();

	/// <summary>
	/// エミッターを追加する(名前は "Emitter_番号" になる).
	/// </summary>
	ParticleEmitterEntity* AddEmitter(const std::string& particleName, const Vector3& position);

	ParticleEmitterEntity* FindEmitter(const std::string& name) const;
	const std::vector<std::unique_ptr<ParticleEmitterEntity>>& GetEmitters() const { return emitters_; };

	// 今のエミッターをシーンの保存ファイルに書き出す.
	bool Save();

	std::string GetFilePath() const { return GetFilePath(sceneName_); };
	static std::string GetFilePath(const std::string& sceneName) { return kDirectoryPath + sceneName + ".json"; };

private:
	EmitterManager() = default;
	~EmitterManager() = default;
	EmitterManager(const EmitterManager&) = delete;
	EmitterManager& operator=(const EmitterManager&) = delete;

	nlohmann::json ToJson() const;
	void FromJson(const nlohmann::json& json);

	// 使われていない "Emitter_番号" の名前.
	std::string MakeUniqueName() const;

private:
	std::string sceneName_;
	std::vector<std::unique_ptr<ParticleEmitterEntity>> emitters_;

	// 最後に保存 / 読み込みした内容(保存していない変更があるかの判定に使う).
	nlohmann::json savedJson_;

	// シーンごとの編集中の状態.
	// 終了ボタンやシーンの切り替えでシーンを作り直しても、保存していない変更が消えないようにする.
	struct SceneState {
		nlohmann::json current; // 編集中の内容.
		nlohmann::json saved;   // 最後に保存 / 読み込みした内容.
	};
	std::map<std::string, SceneState> sceneStates_;

	// 最後の操作の結果の表示.
	std::string message_;
};
