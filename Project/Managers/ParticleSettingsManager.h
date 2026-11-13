#pragma once
#include <string>
#include <vector>
#include "ParticleManager.h"
#include "../externals/nlohmann/json.hpp"

/// <summary>
/// GUIで作るパーティクル(ParticleSettings)を管理する.
/// ・"パーティクル"ウィンドウで追加 / 保存し、Inspectorの各パーティクルのタブで
///   CreateNewParticles(全てのオーバーロード) と SetParticleSize に渡す内容を変えたり、削除したりする.
/// ・変えた内容は次のフレームの最初にParticleManagerのパーティクルに反映する(描画に使っている途中のバッファを消さないため).
/// ・保存すると Resources/Particle/particles.json に書き出し、最初のシーンを作る時に読み込む(Release版でも読み込んで使える).
/// ・作ったパーティクルはコードで作ったものと同じく、名前で SpawnParticles / GetParticles / エミッター から使える.
/// </summary>
class ParticleSettingsManager {
public:
	// 保存先.
	static inline const std::string kDirectoryPath = "Resources/Particle/";
	static inline const std::string kFilePath = kDirectoryPath + "particles.json";

	static ParticleSettingsManager* GetInstance();

	/// <summary>
	/// 保存ファイルから読み込んでパーティクルを作る(2回目以降は何もしない).
	/// テクスチャ / モデルの登録が終わってから呼ぶ(SceneManagerが最初のシーンを作る時に呼ぶ).
	/// </summary>
	void Load();

	/// <summary>
	/// GUIで変えた内容をParticleManagerに反映する(フレームの最初、描画コマンドを積む前に呼ぶ).
	/// </summary>
	void ApplyChanges();

	/// <summary>
	/// "パーティクル"ウィンドウと各パーティクルのInspectorのタブを描画する(ImGuiManager::EndFrameで呼ぶ). Release版では何もしない.
	/// </summary>
	void DrawGui();

	/// <summary>
	/// パーティクルを追加する. 名前が空 / 既に使われている時は追加せずnullptrを返す.
	/// </summary>
	ParticleSettings* Add(const std::string& name);

	// GUIで作ったパーティクルを消す(次のフレームの最初に消える).
	void Remove(const std::string& name);

	ParticleSettings* Find(const std::string& name);

	// 今のパーティクルを保存ファイルに書き出す.
	bool Save();

	// Inspectorのタブの名前.
	static std::string GetInspectorTabName(const std::string& particleName) { return "パーティクル: " + particleName; };

private:
	ParticleSettingsManager() = default;
	~ParticleSettingsManager() = default;
	ParticleSettingsManager(const ParticleSettingsManager&) = delete;
	ParticleSettingsManager& operator=(const ParticleSettingsManager&) = delete;

	// GUIで作ったパーティクル1つ分.
	struct Entry {
		ParticleSettings settings;
		bool needsApply = true;   // 次のフレームの最初に反映するか.
		bool needsRebuild = true; // 形 / テクスチャも作り直すか.
	};

	Entry* FindEntry(const std::string& name);

	nlohmann::json ToJson() const;
	void FromJson(const nlohmann::json& json);

	static nlohmann::json SettingsToJson(const ParticleSettings& settings);
	static ParticleSettings SettingsFromJson(const nlohmann::json& json);

	// 各パーティクルのInspectorのタブ.
	void DrawInspector(Entry& entry);

private:
	std::vector<Entry> entries_;
	// 次のフレームの最初にParticleManagerから消すパーティクル.
	std::vector<std::string> removedNames_;

	bool isLoaded_ = false;

	// 最後に保存 / 読み込みした内容(保存していない変更があるかの判定に使う).
	nlohmann::json savedJson_;

	// 追加する名前の入力欄.
	char newNameBuffer_[64] = "new_particle";
	// 最後の操作の結果の表示.
	std::string message_;
	// テクスチャ / モデルの一覧の絞り込み.
	char filterBuffer_[64] = "";
};
