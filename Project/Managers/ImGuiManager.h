#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <string>
#include <map>
#include <vector>
#include "WinApp.h"
#include "../Engine/Math/Vector2.h"
#include "../Engine/Math/Transform.h"
#include "IScene.h"
#include "nameof.hpp"

/// <summary>
/// ImGuiの初期化・フレーム開始/終了・終了処理と、エンジン用のGUIをまとめて管理する.
/// ・GameGUI        : ゲーム画面(RenderTexture)を表示する"Game"ウィンドウ.
/// ・PlayControlGUI : "Game"ウィンドウの上部に中央ぞろえで並ぶ 再生(終了) / 一時停止 ボタン.
/// ・SceneSelectGUI : SceneNameを選んで読み込む"シーン切り替え"ウィンドウ.
/// ・Hierarchy      : Begin(name)で作られたタブの一覧. 選んだタブの中身がInspectorに出る.
/// ・Inspector      : Begin(name)〜End() で囲んだImGuiのうち、Hierarchyで選ばれたタブの中身を表示する.
/// </summary>
class ImGuiManager {
public:
	/// <summary>
	/// ゲームの実行状態(Unityの再生モードと同じ考え方).
	/// </summary>
	enum class PlayState {
		kStop,  // 停止中(再生前の状態。更新処理は行わない).
		kPlay,  // 再生中.
		kPause, // 一時停止中(状態は保持したまま更新処理を止める).
	};

	static ImGuiManager* GetInstance();

	/// <summary>
	/// ImGuiの初期化.
	/// </summary>
	void Initialize(WinApp* winApp, ID3D12Device* device, uint32_t bufferCount, DXGI_FORMAT rtvFormat, ID3D12CommandQueue* commandQueue);

	/// <summary>
	/// エンジンのGUIで使う画像(Engine/UI)を読み込む.
	/// TextureManagerが使えるようになってから呼ぶ. Release版(USE_IMGUI無し)では何もしない.
	/// </summary>
	void LoadTextures();

	/*=============================================================
	Inspector(Begin / End で囲んだImGuiをタブにまとめて表示する)
	=============================================================*/
	/// <summary>
	/// "Hierarchy"ウィンドウに name のタブを追加し、End()までのImGuiをそのタブの中身にする.
	/// Hierarchyで選ばれているタブの中身が"Inspector"ウィンドウに表示される.
	/// 同じ name で何度呼んでも同じタブに続けて描画される.
	/// タブが選ばれていない時は中身を描画しない(ImGuiの関数は呼んでも何も起きない).
	/// </summary>
	/// <param name="name">タブの名前</param>
	/// <returns>タブの中身が表示されているか</returns>
	bool Begin(const std::string& name);

	/// <summary>
	/// Begin()で開いたタブを閉じる(Beginの戻り値に関係なく必ず呼ぶ).
	/// </summary>
	void End();

	/// <summary>
	/// name のタブを選んだ状態にする(Hierarchyでタブをクリックした時と同じ).
	/// </summary>
	void SelectInspectorTab(const std::string& name) { selectedInspectorTab_ = name; };

	/// <summary>
	/// ImGuiのフレーム開始(GameSystem::BeginFrameで呼ぶ).
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// エンジンのGUIを描画し、ImGuiの描画コマンドを積む(GameSystem::EndFrameで呼ぶ).
	/// </summary>
	void EndFrame(ID3D12GraphicsCommandList* commandList);

	/// <summary>
	/// ImGuiの終了処理.
	/// </summary>
	void Finalize();

	/*=============================================================
	GameGUI(ゲーム画面ウィンドウ)
	=============================================================*/
	// "Game"ウィンドウに表示するテクスチャ(RenderTextureのSRV)を設定する.
	void SetGameViewTexture(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) { gameViewTexture_ = textureHandle; };
	// "Game"ウィンドウ内のゲーム画面の表示位置/サイズ(スクリーン座標).
	const Vector2& GetGameViewPos() const { return gameViewPos_; };
	const Vector2& GetGameViewSize() const { return gameViewSize_; };
	bool IsGameViewFocused() const { return isGameViewFocused_; };
	bool IsGameViewHovered() const { return isGameViewHovered_; };

	/*=============================================================
	PlayControlGUI(再生 / 一時停止 / 終了)
	=============================================================*/
	PlayState GetPlayState() const { return playState_; };
	// ゲームの更新処理を行ってよいか(再生中のみtrue).
	bool IsPlaying() const { return playState_ == PlayState::kPlay; };

	// 停止中 → 再生 / 一時停止中 → 再開.
	void Play();
	// 再生中 → 一時停止.
	void Pause();
	/// <summary>
	/// UI(Renderer::DrawSpriteで描画するもの)を描画してよいか.
	/// 再生中はいつもtrue. 停止中 / 一時停止中は"Game"ウィンドウのUIボタンで切り替える.
	/// </summary>
	bool IsGameUIVisible() const { return IsPlaying() || isUIVisibleWhenNotPlaying_; };
	void SetUIVisibleWhenNotPlaying(bool isVisible) { isUIVisibleWhenNotPlaying_ = isVisible; };

	// 再生中 ⇔ 一時停止中 を切り替える(停止中は何もしない).
	void TogglePause();
	// 再生中 / 一時停止中 → 停止(再生前の状態に戻すよう要求する).
	void Stop();

	// 停止状態から再生が開始されたか(呼ぶとフラグは下りる).
	bool ConsumePlayStarted();
	// 終了ボタンが押されたか(呼ぶとフラグは下りる).
	bool ConsumeStopRequested();

	/*=============================================================
	SceneSelectGUI(シーンの切り替え)
	=============================================================*/
	// 今読み込まれているシーン(選択欄の初期値にする。SceneManagerが設定する).
	void SetCurrentSceneName(SceneName name) { selectedSceneName_ = name; };
	// 読み込みボタンが押されたか. 押されていればoutNameに読み込むシーンが入る(呼ぶとフラグは下りる).
	bool ConsumeLoadSceneRequest(SceneName& outName);

	/*=============================================================
	Gizmo(ImGuizmo. "Game"ウィンドウの上でTransformを操作する)
	=============================================================*/
	// ギズモで操作できる種類(組み合わせて指定する).
	static constexpr uint32_t kGizmoTranslate = 1u << 0; // 移動.
	static constexpr uint32_t kGizmoRotate = 1u << 1;    // 回転.
	static constexpr uint32_t kGizmoScale = 1u << 2;     // 拡縮.
	static constexpr uint32_t kGizmoAll = kGizmoTranslate | kGizmoRotate | kGizmoScale;

	enum class GizmoOperation {
		kTranslate,
		kRotate,
		kScale,
	};

	/// <summary>
	/// このフレームの"Game"ウィンドウに transform を操作するギズモを出す.
	/// 出している間は毎フレーム呼ぶ(呼ばなかったフレームは出ない).
	/// 操作した結果はこのフレームの最後(EndFrame)で transform に書き込まれるので、transformはフレームの最後まで残しておくこと.
	/// 親があるTransformはワールドで操作し、親から見た値に直して書き込む. Release版では何もしない.
	/// </summary>
	/// <param name="id">ギズモごとに別の名前</param>
	/// <param name="transform">操作するTransform</param>
	/// <param name="operations">操作できる種類(kGizmoTranslate | kGizmoScale など)</param>
	void Gizmo(const std::string& id, Transform& transform, uint32_t operations = kGizmoAll);

	// ギズモにマウスが乗っている / 操作中か(カメラ操作などと重ならないように使う).
	bool IsUsingGizmo() const { return isGizmoUsing_; };

	GizmoOperation GetGizmoOperation() const { return gizmoOperation_; };
	void SetGizmoOperation(GizmoOperation operation) { gizmoOperation_ = operation; };

private:
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	void DrawGameViewWindow();

	// "Game"ウィンドウの上部に 再生 / 一時停止 ボタンを中央ぞろえで描画する.
	void DrawPlayControlBar();

	void DrawSceneSelectWindow();

	// Begin(name)で作られたタブの一覧("Hierarchy"ウィンドウ)を描画する.
	void DrawHierarchyWindow();

	// Begin / End が1回も呼ばれなかったフレームでも"Inspector"ウィンドウを出しておく.
	void DrawInspectorWindow();

	// "Game"ウィンドウのゲーム画面の上にギズモと操作の切り替えボタンを描画する.
	void DrawGizmos(const Vector2& imagePos, const Vector2& imageSize);

	// "Game"ウィンドウの上部の右側に UIの表示 / デバッグカメラ の切り替えボタンを描画する.
	void DrawViewOptionButtons();

	/// <summary>
	/// エディターの設定(停止中 / 一時停止中のUIの表示など)を imgui.ini に保存 / 読み込みできるようにする.
	/// </summary>
	void RegisterEditorSettingsHandler();

	/// <summary>
	/// 日本語を表示できるフォント(Windowsのメイリオなど)を読み込む.
	/// </summary>
	void LoadJapaneseFont();

private:
	// ImGuiの文字の大きさ(px).
	static inline const float kFontSize_ = 16.0f;
	// 再生 / 一時停止 ボタンの画像の表示サイズ(px).
	static inline const float kPlayControlIconSize_ = 24.0f;

	// GameGUI.
	D3D12_GPU_DESCRIPTOR_HANDLE gameViewTexture_{};
	Vector2 gameViewPos_{};
	Vector2 gameViewSize_{};
	bool isGameViewFocused_ = false;
	bool isGameViewHovered_ = false;

	// PlayControlGUI.
#ifdef USE_IMGUI
	// エディタ(ImGui)がある時はUnityのように停止状態から始める.
	PlayState playState_ = PlayState::kStop;
#else
	// ImGuiが無い(Release)時はボタンが押せないので最初から再生する.
	PlayState playState_ = PlayState::kPlay;
#endif // USE_IMGUI
#ifdef USE_IMGUI
	// ボタンの画像(Engine/UI/play.png, pause.png, stop.png).
	D3D12_GPU_DESCRIPTOR_HANDLE playIcon_{};
	D3D12_GPU_DESCRIPTOR_HANDLE pauseIcon_{};
	D3D12_GPU_DESCRIPTOR_HANDLE pauseInactiveIcon_{};
	D3D12_GPU_DESCRIPTOR_HANDLE stopIcon_{};
#endif // USE_IMGUI
	// 停止中 / 一時停止中にUIを表示するか(imgui.ini に保存して次回の起動でも同じにする).
	bool isUIVisibleWhenNotPlaying_ = true;
	bool isDebugUIVisibleWhenNotPlaying_ = true;
	bool isPlayStarted_ = false;
	bool isStopRequested_ = false;

	// SceneSelectGUI.
	SceneName selectedSceneName_ = SceneName::kTitleScene;
	bool isLoadSceneRequested_ = false;

	// Inspector.
	// Begin〜Endの中身をどこに描画しているか.
	enum class InspectorTabState {
		kNone,     // Begin〜Endの外.
		kVisible,  // 選ばれているタブ(Inspectorに描画する).
		kHidden,   // タブが選ばれていない(見えない場所に描画して捨てる).
	};
	InspectorTabState inspectorTabState_ = InspectorTabState::kNone;
	// このフレームでBeginされたタブの名前(Beginされた順. Hierarchyに縦に並べる).
	std::vector<std::string> inspectorTabOrder_;
	// このフレームで各タブがBeginされた回数(同じタブの2回目以降はIDを分ける).
	std::map<std::string, int> inspectorTabBeginCounts_;
	// 選ばれているタブ.
	std::string selectedInspectorTab_;

	// Gizmo.
	struct GizmoTarget {
		std::string id;
		Transform* transform = nullptr;
		uint32_t operations = kGizmoAll;
	};
	// このフレームでGizmo()が呼ばれたTransform.
	std::vector<GizmoTarget> gizmoTargets_;
	GizmoOperation gizmoOperation_ = GizmoOperation::kTranslate;
	// ワールドの向きで操作するか(falseならTransformの向き).
	bool isGizmoWorld_ = true;
	bool isGizmoUsing_ = false;
};
