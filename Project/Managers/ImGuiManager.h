#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cstdint>
#include "WinApp.h"
#include "../Engine/Math/Vector2.h"

/// <summary>
/// ImGuiの初期化・フレーム開始/終了・終了処理と、エンジン用のGUIをまとめて管理する.
/// ・GameGUI        : ゲーム画面(RenderTexture)を表示する"Game"ウィンドウ.
/// ・PlayControlGUI : 再生 / 一時停止 / 終了 ボタンを持つ"PlayControl"ウィンドウ.
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

	/// <summary>
	/// ImGuiのフレーム開始(GameSystem::BeginFrameで呼ぶ).
	/// </summary>
	void Begin();

	/// <summary>
	/// エンジンのGUIを描画し、ImGuiの描画コマンドを積む(GameSystem::EndFrameで呼ぶ).
	/// </summary>
	void End(ID3D12GraphicsCommandList* commandList);

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

private:
	ImGuiManager() = default;
	~ImGuiManager() = default;
	ImGuiManager(const ImGuiManager&) = delete;
	ImGuiManager& operator=(const ImGuiManager&) = delete;

	void DrawGameViewWindow();

	void DrawPlayControlWindow();

private:
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
};
