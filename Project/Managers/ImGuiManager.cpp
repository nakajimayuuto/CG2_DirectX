#include "ImGuiManager.h"
#include <algorithm>
#include <filesystem>
#include "SrvManager.h"
#include "../Engine/Environment/Environment.h"

#ifdef USE_IMGUI
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/backends/imgui_impl_dx12.h"
#include "../externals/imgui/backends/imgui_impl_win32.h"
#include "TextureManager.h"
#include "../externals/imgui/imgui_internal.h"
#include "../externals/magic_enum/magic_enum.hpp"
#endif // USE_IMGUI
#include <cassert>
#include "ParticleNodeEditor.h"

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

#ifdef USE_IMGUI
namespace {
	// Inspectorのウィンドウ名とタブバーのID.
	const char* const kInspectorWindowName = "Inspector";
	const char* const kInspectorTabBarId = "##InspectorTabs";
	// タブが選ばれていない時に中身を捨てるためのウィンドウ.
	const char* const kInspectorHiddenWindowName = "##InspectorHidden";
}
#endif // USE_IMGUI

bool ImGuiManager::Begin(const std::string& name) {
#ifdef USE_IMGUI
	// Begin〜Endの入れ子はできない(Endの呼び忘れ).
	assert(inspectorTabState_ == InspectorTabState::kNone);

	inspectorTabState_ = InspectorTabState::kHidden;
	isInspectorTabBarOpen_ = false;

	// 同じ名前のウィンドウ / タブバーは、同じフレームで何度Beginしても続きに追加される.
	const bool isWindowVisible = ImGui::Begin(kInspectorWindowName);
	if (isWindowVisible && ImGui::BeginTabBar(kInspectorTabBarId, ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll)) {
		isInspectorTabBarOpen_ = true;

		auto it = inspectorTabs_.find(name);
		if (it == inspectorTabs_.end()) {
			// このフレームで初めての名前 → タブを追加する.
			const bool isTabVisible = ImGui::BeginTabItem(name.c_str());
			inspectorTabs_[name] = isTabVisible;
			if (isTabVisible) {
				inspectorTabState_ = InspectorTabState::kTabItem;
			}
		} else if (it->second) {
			// 同じ名前の2回目以降 → タブは増やさず、表示中なら1回目の続きに描画する.
			// (同じタブをもう一度BeginTabItemするとIDが重複するので、IDだけ分けておく).
			inspectorTabState_ = InspectorTabState::kAppend;
			ImGui::PushID(name.c_str());
		}

		if (inspectorTabState_ != InspectorTabState::kHidden && hasInspectorContentCursor_) {
			// タブバーを開き直すとカーソルがタブの直下に戻るので、前回描画した所の続きに移動する.
			// (SetCursorScreenPosだと何も描画しなかった時にImGuiの警告が出るので直接動かす).
			ImGui::GetCurrentWindow()->DC.CursorPos = ImVec2(inspectorContentCursor_.x, inspectorContentCursor_.y);
		}
	}

	if (inspectorTabState_ == InspectorTabState::kHidden) {
		// タブが選ばれていない / Inspectorが隠れている時は、画面外の折りたたんだウィンドウに描画させて捨てる.
		// (折りたたまれたウィンドウの中ではImGuiの関数は何もしないので、呼ぶ側はifで囲まなくてよい).
		ImGui::SetNextWindowPos(ImVec2(-10000.0f, -10000.0f), ImGuiCond_Always);
		ImGui::SetNextWindowCollapsed(true, ImGuiCond_Always);
		ImGui::Begin(kInspectorHiddenWindowName, nullptr,
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		return false;
	}

	return true;
#else
	(void)name;
	return false;
#endif // USE_IMGUI
}

void ImGuiManager::End() {
#ifdef USE_IMGUI
	// Beginを呼ばずにEndを呼んでいる.
	assert(inspectorTabState_ != InspectorTabState::kNone);

	switch (inspectorTabState_) {
	case InspectorTabState::kTabItem:
	case InspectorTabState::kAppend:
	{
		// 次に同じ名前でBeginされた時の描画開始位置を覚えておく.
		const ImVec2 cursor = ImGui::GetCurrentWindow()->DC.CursorPos;
		inspectorContentCursor_ = { cursor.x, cursor.y };
		hasInspectorContentCursor_ = true;

		if (inspectorTabState_ == InspectorTabState::kTabItem) {
			ImGui::EndTabItem();
		} else {
			ImGui::PopID();
		}
		break;
	}
	case InspectorTabState::kHidden:
		// 中身を捨てるためのウィンドウを閉じる.
		ImGui::End();
		break;
	default:
		break;
	}

	if (isInspectorTabBarOpen_) {
		ImGui::EndTabBar();
	}
	// "Inspector"ウィンドウはBeginの戻り値に関係なく必ずEndする.
	ImGui::End();

	inspectorTabState_ = InspectorTabState::kNone;
	isInspectorTabBarOpen_ = false;
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

	// 日本語を表示できるフォントを読み込む.
	LoadJapaneseFont();
	io.Fonts->Build();
#else
	(void)winApp; (void)device; (void)bufferCount; (void)rtvFormat; (void)commandQueue;
#endif // USE_IMGUI
}

void ImGuiManager::LoadJapaneseFont() {
#ifdef USE_IMGUI
	ImGuiIO& io = ImGui::GetIO();

	// Windowsに標準で入っている日本語フォント(上から順に、見つかったものを使う).
	// ImGui 1.92以降は使った文字だけをその都度読み込むので、文字の範囲(GlyphRanges)の指定は不要.
	static const char* const kFontPaths[] = {
		"C:/Windows/Fonts/meiryo.ttc",   // メイリオ.
		"C:/Windows/Fonts/YuGothM.ttc",  // 游ゴシック.
		"C:/Windows/Fonts/msgothic.ttc", // MS ゴシック.
	};

	for (const char* path : kFontPaths) {
		if (!std::filesystem::exists(path)) {
			continue;
		}
		if (io.Fonts->AddFontFromFileTTF(path, kFontSize_)) {
			return;
		}
	}

	// 日本語フォントが見つからなかった場合はImGuiの標準フォントを使う(日本語は表示されない).
	io.Fonts->AddFontDefault();
#endif // USE_IMGUI
}

void ImGuiManager::BeginFrame() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	// Inspectorのタブはフレームごとに追加し直す.
	inspectorTabs_.clear();
	hasInspectorContentCursor_ = false;
	inspectorTabState_ = InspectorTabState::kNone;
#endif // USE_IMGUI
}

void ImGuiManager::EndFrame(ID3D12GraphicsCommandList* commandList) {
#ifdef USE_IMGUI
	// エンジンのGUI.
	DrawGameViewWindow();
	DrawSceneSelectWindow();
	DrawInspectorWindow();
	// パーティクルの動きを作るノードエディター.
	ParticleNodeEditor::GetInstance()->Draw();

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
	// ノードエディターはImGuiより先に終了する.
	ParticleNodeEditor::GetInstance()->Finalize();
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

bool ImGuiManager::ConsumeLoadSceneRequest(SceneName& outName) {
	if (!isLoadSceneRequested_) {
		return false;
	}
	isLoadSceneRequested_ = false;
	outName = selectedSceneName_;
	return true;
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

	// ゲーム画面の上に 再生 / 一時停止 ボタンを中央ぞろえで並べる.
	DrawPlayControlBar();

	// アスペクト比を保ったまま、残りの領域に収まる最大サイズを求める.
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
PlayControlGUI("Game"ウィンドウの上部に表示する)
=============================================================*/
void ImGuiManager::DrawPlayControlBar() {
	// 現在の状態のボタンを色付けする.
	const ImVec4 activeColor = ImVec4(0.20f, 0.45f, 0.80f, 1.0f);
	// ボタンの画像の表示サイズ(画像は32x32).
	const ImVec2 iconSize = ImVec2(kPlayControlIconSize_, kPlayControlIconSize_);
	// ボタンの上下の余白.
	const float kBarPaddingY = 4.0f;

	// ボタン2つ分の幅から、中央ぞろえになる開始位置を求める.
	const ImGuiStyle& style = ImGui::GetStyle();
	const float buttonWidth = iconSize.x + style.FramePadding.x * 2.0f;
	const float barWidth = buttonWidth * 2.0f + style.ItemSpacing.x;
	ImVec2 cursor = ImGui::GetCursorPos();
	cursor.x += (std::max)(0.0f, (ImGui::GetContentRegionAvail().x - barWidth) * 0.5f);
	cursor.y += kBarPaddingY;
	ImGui::SetCursorPos(cursor);

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

	// ボタンの下に余白をあけてからゲーム画面を置く.
	ImGui::Dummy(ImVec2(0.0f, kBarPaddingY));
}

/*=============================================================
SceneSelectGUI
=============================================================*/
void ImGuiManager::DrawSceneSelectWindow() {
	ImGui::SetNextWindowSize(ImVec2(300.0f, 60.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("シーン切り替え", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	// 選択欄と読み込みボタンを横に並べる(ボタンの幅を残して選択欄を広げる).
	const float buttonWidth = ImGui::CalcTextSize("読み込み").x + ImGui::GetStyle().FramePadding.x * 2.0f;
	ImGui::SetNextItemWidth((std::max)(80.0f, ImGui::GetContentRegionAvail().x - buttonWidth - ImGui::GetStyle().ItemSpacing.x));

	// SceneNameの列挙型から選択する(列挙子を増やすと自動でここにも並ぶ).
	if (ImGui::BeginCombo("##SceneSelect", magic_enum::enum_name(selectedSceneName_).data())) {
		for (SceneName name : magic_enum::enum_values<SceneName>()) {
			const bool isSelected = name == selectedSceneName_;
			if (ImGui::Selectable(magic_enum::enum_name(name).data(), isSelected)) {
				selectedSceneName_ = name;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();

	// 読み込み(実際の読み込みは次フレームの更新の最初にSceneManagerが行う).
	if (ImGui::Button("読み込み")) {
		isLoadSceneRequested_ = true;
	}

	ImGui::End();
}

/*=============================================================
Inspector
=============================================================*/
void ImGuiManager::DrawInspectorWindow() {
	// Begin〜Endが閉じられていない(Endの呼び忘れ).
	assert(inspectorTabState_ == InspectorTabState::kNone);

	// このフレームでBeginされていてもいなくても、ウィンドウ自体は毎フレーム出しておく(同じ名前なので続きに追加される).
	if (ImGui::Begin(kInspectorWindowName) && inspectorTabs_.empty()) {
		ImGui::TextDisabled("ImGuiManager::Begin(\"名前\") 〜 End() で囲んだ");
		ImGui::TextDisabled("ImGuiがここにタブで表示されます.");
	}
	ImGui::End();
}
#endif // USE_IMGUI
