#include "ImGuiManager.h"
#include <algorithm>
#include "SrvManager.h"
#include "../Engine/Environment/Environment.h"

#ifdef USE_IMGUI
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/backends/imgui_impl_dx12.h"
#include "../externals/imgui/backends/imgui_impl_win32.h"
#include "TextureManager.h"
#endif // USE_IMGUI

ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::LoadTextures() {
#ifdef USE_IMGUI
	// エンジンのGUI用の画像(Engine/UI). Release版ではImGuiを使わないので読み込まない.
	// ゲーム側の"pause"(Resources/UI/pause.png)と名前が被らないよう"engine_ui_"を付ける.
	TextureManager* textureManager = TextureManager::GetInstance();
	textureManager->RegisterTexture("engine_ui_play", "Engine/UI/play.png", false);
	playIcon_ = textureManager->GetTextureInfo("engine_ui_play").textureSrvHandlesGPU;
	textureManager->RegisterTexture("engine_ui_pause", "Engine/UI/pause.png", false);
	pauseIcon_ = textureManager->GetTextureInfo("engine_ui_pause").textureSrvHandlesGPU;
	textureManager->RegisterTexture("engine_ui_pause_inactive", "Engine/UI/pause_inactive.png", false);
	pauseInactiveIcon_ = textureManager->GetTextureInfo("engine_ui_pause_inactive").textureSrvHandlesGPU;
	textureManager->RegisterTexture("engine_ui_stop", "Engine/UI/stop.png", false);
	stopIcon_ = textureManager->GetTextureInfo("engine_ui_stop").textureSrvHandlesGPU;
#endif // USE_IMGUI
}

void ImGuiManager::Initialize(WinApp* winApp, ID3D12Device* device, uint32_t bufferCount, DXGI_FORMAT rtvFormat, ID3D12CommandQueue* commandQueue) {
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	// Docking機能を有効化.
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHWND());
	ImGui_ImplDX12_InitInfo info;
	info.Device = device;
	info.NumFramesInFlight = static_cast<int>(bufferCount);
	info.RTVFormat = rtvFormat;
	info.CommandQueue = commandQueue;
	info.SrvDescriptorHeap = SrvManager::GetInstance()->GetDescriptorHeap().Get();
	info.LegacySingleSrvCpuDescriptor = SrvManager::GetInstance()->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	info.LegacySingleSrvGpuDescriptor = SrvManager::GetInstance()->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	ImGui_ImplDX12_Init(&info);
	io.Fonts->Build();
#else
	(void)winApp; (void)device; (void)bufferCount; (void)rtvFormat; (void)commandQueue;
#endif // USE_IMGUI
}

void ImGuiManager::Begin() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();
#endif // USE_IMGUI
}

void ImGuiManager::End(ID3D12GraphicsCommandList* commandList) {
#ifdef USE_IMGUI
	// エンジンのGUI.
	DrawGameViewWindow();
	DrawPlayControlWindow();

	// ImGuiの内部コマンドを生成する.
	ImGui::Render();

	// 実際のcommandListにImGuiの描画コマンドを積む.
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#else
	(void)commandList;
#endif // USE_IMGUI
}

void ImGuiManager::Finalize() {
#ifdef USE_IMGUI
	// 初期化と逆順に行う.
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif // USE_IMGUI
}

/*=============================================================
PlayControl
=============================================================*/
void ImGuiManager::Play() {
	if (playState_ == PlayState::kStop) {
		// 停止状態からの再生開始(再生前の状態を記録してもらう).
		isPlayStarted_ = true;
	}
	playState_ = PlayState::kPlay;
}

void ImGuiManager::Pause() {
	if (playState_ != PlayState::kPlay) {
		return;
	}
	playState_ = PlayState::kPause;
}

void ImGuiManager::TogglePause() {
	if (playState_ == PlayState::kPlay) {
		playState_ = PlayState::kPause;
	} else if (playState_ == PlayState::kPause) {
		playState_ = PlayState::kPlay;
	}
}

void ImGuiManager::Stop() {
	if (playState_ == PlayState::kStop) {
		return;
	}
	playState_ = PlayState::kStop;
	// 実際に状態を戻すのは次フレームの更新処理の最初で行う.
	// (このフレームの描画コマンドが参照しているリソースを、実行前に破棄しないため).
	isStopRequested_ = true;
}

bool ImGuiManager::ConsumePlayStarted() {
	bool result = isPlayStarted_;
	isPlayStarted_ = false;
	return result;
}

bool ImGuiManager::ConsumeStopRequested() {
	bool result = isStopRequested_;
	isStopRequested_ = false;
	return result;
}

#ifdef USE_IMGUI
/*=============================================================
GameGUI
=============================================================*/
void ImGuiManager::DrawGameViewWindow() {
	// ゲーム画面がウィンドウいっぱいに表示されるよう余白をなくす.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PopStyleVar();

	isGameViewFocused_ = ImGui::IsWindowFocused();
	isGameViewHovered_ = ImGui::IsWindowHovered();

	// アスペクト比を保ったまま、ウィンドウに収まる最大サイズを求める.
	const ImVec2 avail = ImGui::GetContentRegionAvail();
	const float textureWidth = static_cast<float>(Environment::GetInstance()->GetWindowSize().width);
	const float textureHeight = static_cast<float>(Environment::GetInstance()->GetWindowSize().height);
	const float textureAspect = textureWidth / textureHeight;

	ImVec2 imageSize = avail;
	if (avail.x > 0.0f && avail.y > 0.0f) {
		if (avail.x / avail.y > textureAspect) {
			imageSize.x = avail.y * textureAspect;
		} else {
			imageSize.y = avail.x / textureAspect;
		}

		// 中央に寄せる.
		ImVec2 cursor = ImGui::GetCursorPos();
		cursor.x += (avail.x - imageSize.x) * 0.5f;
		cursor.y += (avail.y - imageSize.y) * 0.5f;
		ImGui::SetCursorPos(cursor);

		const ImVec2 screenPos = ImGui::GetCursorScreenPos();
		gameViewPos_ = { screenPos.x, screenPos.y };
		gameViewSize_ = { imageSize.x, imageSize.y };

		ImGui::Image(static_cast<ImTextureID>(gameViewTexture_.ptr), imageSize);
	}

	ImGui::End();
}

/*=============================================================
PlayControlGUI
=============================================================*/
void ImGuiManager::DrawPlayControlWindow() {
	ImGui::SetNextWindowSize(ImVec2(180.0f, 70.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("PlayControl", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	// 現在の状態のボタンを色付けする.
	const ImVec4 activeColor = ImVec4(0.20f, 0.45f, 0.80f, 1.0f);
	// ボタンの画像の表示サイズ(画像は32x32).
	// ドッキングでウィンドウが低くなっている時は、ボタンが切れないよう高さに合わせて縮める.
	const float kIconMaxSize = 32.0f;
	const float kIconMinSize = 12.0f;
	const float fitSize = ImGui::GetContentRegionAvail().y - ImGui::GetStyle().FramePadding.y * 2.0f;
	const float iconLength = (std::max)(kIconMinSize, (std::min)(kIconMaxSize, fitSize));
	const ImVec2 iconSize = ImVec2(iconLength, iconLength);

	// 再生 / 終了ボタン.
	// 停止中は再生(play.png)、再生後(一時停止中も含む)は終了(stop.png)の画像に置き換わる.
	const bool isStopped = playState_ == PlayState::kStop;
	{
		if (!isStopped) { ImGui::PushStyleColor(ImGuiCol_Button, activeColor); }
		const D3D12_GPU_DESCRIPTOR_HANDLE icon = isStopped ? playIcon_ : stopIcon_;
		// 画像が変わってもボタンのIDは同じにする.
		const bool isPressed = ImGui::ImageButton("PlayStop", static_cast<ImTextureID>(icon.ptr), iconSize);
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip(isStopped ? "Play" : "Stop");
		}
		if (isPressed) {
			if (isStopped) {
				Play();
			} else {
				// 再生前の状態に戻る.
				Stop();
			}
		}
		if (!isStopped) { ImGui::PopStyleColor(); }
	}

	ImGui::SameLine();

	// 一時停止ボタン(押すたびに 再生 ⇔ 一時停止 を切り替える。停止中は押せない).
	{
		const bool isPaused = playState_ == PlayState::kPause;
		const D3D12_GPU_DESCRIPTOR_HANDLE pauseIcon = isStopped ? pauseInactiveIcon_ : pauseIcon_;
		if (isPaused) { ImGui::PushStyleColor(ImGuiCol_Button, activeColor); }
		ImGui::BeginDisabled(playState_ == PlayState::kStop);
		if (ImGui::ImageButton("Pause", static_cast<ImTextureID>(pauseIcon.ptr), iconSize)) {
			TogglePause();
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
			ImGui::SetTooltip(isPaused ? "Resume" : "Pause");
		}
		if (isPaused) { ImGui::PopStyleColor(); }
	}

	switch (playState_) {
	case PlayState::kStop:
		ImGui::Text("State : Stop");
		break;
	case PlayState::kPlay:
		ImGui::Text("State : Play");
		break;
	case PlayState::kPause:
		ImGui::Text("State : Pause");
		break;
	}

	ImGui::End();
}
#endif // USE_IMGUI
