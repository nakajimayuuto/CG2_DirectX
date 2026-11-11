#pragma once
#include <string>
#include <memory>
#include "GuiNode.h"
#include "../Engine/Math/Vector3.h"

class ParticleNodeMove;

/// <summary>
/// パーティクルの動きをノードで作る"パーティクルエディター"ウィンドウ.
/// ・作った動きは Resources/Node/Particle/名前.json に保存する.
/// ・保存した動きは ParticleManager::CreateNewParticles(名前, テクスチャ, ビルボード, "動きの名前") で使える.
/// ・再生中はゲーム画面にプレビューを出せる(編集した内容がすぐ反映される).
/// Release版(USE_IMGUI無し)では何もしない.
/// </summary>
class ParticleNodeEditor {
public:
	static ParticleNodeEditor* GetInstance();

	/// <summary>
	/// ウィンドウを描画する(ImGuiManager::EndFrameで呼ぶ).
	/// </summary>
	void Draw();

	/// <summary>
	/// 終了処理(ImGuiの終了前に呼ぶ).
	/// </summary>
	void Finalize();

private:
	ParticleNodeEditor() = default;
	~ParticleNodeEditor() = default;
	ParticleNodeEditor(const ParticleNodeEditor&) = delete;
	ParticleNodeEditor& operator=(const ParticleNodeEditor&) = delete;

	void Initialize();

	// 新しいグラフにする(生成時 / 毎フレーム / パーティクル ノードを置く).
	void NewGraph();
	bool SaveGraph();
	bool LoadGraph(const std::string& name);

	void DrawToolbar();
	void DrawPreviewSettings();
	// プレビュー用のパーティクルを出す.
	void UpdatePreview();

private:
	// プレビューに使うパーティクルの名前(ParticleManagerに登録する).
	static inline const std::string kPreviewParticleName = "__ParticleNodeEditorPreview";

	bool isInitialized_ = false;
	GuiNode graph_;
	// 編集中の動きの名前と、保存してから変更があるか.
	char nameBuffer_[128] = "new_particle";
	bool isDirty_ = false;
	// 読み込み欄で選んでいる名前.
	std::string selectedLoadName_;
	// 最後の操作の結果の表示.
	std::string message_;

	// プレビュー.
	std::shared_ptr<ParticleNodeMove> previewMove_;
	// ONの間は再生中にゲーム画面へパーティクルを出し続けるので、最初はOFFにしておく.
	bool isPreviewEnabled_ = false;
	Vector3 previewPosition_{ 0.0f, 0.0f, 0.0f };
	int previewCount_ = 3;
	float previewInterval_ = 0.1f;
	float previewSize_ = 0.5f;
	float previewTimer_ = 0.0f;
	char previewTexture_[64] = "effect_plane";
	// プレビュー用のパーティクルを作った時のテクスチャ.
	std::string appliedPreviewTexture_;
};
