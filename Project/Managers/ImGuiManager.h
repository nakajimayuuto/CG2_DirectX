#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include <string>
#include <map>
#include "WinApp.h"
#include "../Engine/Math/Vector2.h"
#include "IScene.h"
#include "nameof.hpp"

/// <summary>
/// ImGuiの初期化・フレーム開始/終了・終了処理と、エンジン用のGUIをまとめて管理する.
/// ・GameGUI        : ゲーム画面(RenderTexture)を表示する"Game"ウィンドウ.
/// ・PlayControlGUI : "Game"ウィンドウの上部に中央ぞろえで並ぶ 再生(終了) / 一時停止 ボタン.
/// ・SceneSelectGUI : SceneNameを選んで読み込む"シーン切り替え"ウィンドウ.
/// ・Inspector      : Begin(name)〜End() で囲んだImGuiを name のタブにまとめる"Inspector"ウィンドウ.
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
	/// "Inspector"ウィンドウに name のタブを追加し、End()までのImGuiをそのタブの中に描画する.
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

private:
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	void DrawGameViewWindow();

	// "Game"ウィンドウの上部に 再生 / 一時停止 ボタンを中央ぞろえで描画する.
	void DrawPlayControlBar();

	void DrawSceneSelectWindow();

	// Begin / End が1回も呼ばれなかったフレームでも"Inspector"ウィンドウを出しておく.
	void DrawInspectorWindow();

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
	bool isPlayStarted_ = false;
	bool isStopRequested_ = false;

	// SceneSelectGUI.
	SceneName selectedSceneName_ = SceneName::kTitleScene;
	bool isLoadSceneRequested_ = false;

	// Inspector.
	// Begin〜Endの中身をどこに描画しているか.
	enum class InspectorTabState {
		kNone,     // Begin〜Endの外.
		kTabItem,  // このフレームで初めてのタブ(BeginTabItemの中).
		kAppend,   // 同じ名前の2回目以降(1回目の続きに描画).
		kHidden,   // タブが選ばれていない(見えない場所に描画して捨てる).
	};
	InspectorTabState inspectorTabState_ = InspectorTabState::kNone;
	// Begin()でタブバーを開けたか(Endで閉じる必要があるか).
	bool isInspectorTabBarOpen_ = false;
	// このフレームで追加されたタブと、その中身が表示されているか.
	std::map<std::string, bool> inspectorTabs_;
	// 表示中のタブの中身をどこまで描画したか(同じ名前のBeginの続きをここから描く).
	Vector2 inspectorContentCursor_{};
	bool hasInspectorContentCursor_ = false;
};
