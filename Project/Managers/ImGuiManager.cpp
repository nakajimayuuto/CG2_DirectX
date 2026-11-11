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
#include "../externals/imguizmo/ImGuizmo.h"
#endif // USE_IMGUI
#include <cassert>
#include "ParticleNodeEditor.h"
#include "ParticleManager.h"
#include "EmitterManager.h"
#include "../Engine/Renderer/Camera.h"

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
			// このフレームで初めての名前 → タブを追加する(SelectInspectorTabで指定されていれば選ぶ).
			ImGuiTabItemFlags tabFlags = ImGuiTabItemFlags_None;
			if (name == requestedInspectorTab_) {
				tabFlags |= ImGuiTabItemFlags_SetSelected;
				requestedInspectorTab_.clear();
			}
			const bool isTabVisible = ImGui::BeginTabItem(name.c_str(), nullptr, tabFlags);
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
	// ImGui(1.92以降)はフォントの文字が増えるとテクスチャを作り直し、その間は古いものと新しいものの2つ以上のSRVを使う.
	// そのため1つしか使えない Legacy の指定ではなく、SrvManagerから割り当てる.
	info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* outCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE* outGpuHandle) {
		SrvManager::GetInstance()->AllocateImGuiDescriptor(outCpuHandle, outGpuHandle);
	};
	info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE) {
		SrvManager::GetInstance()->FreeImGuiDescriptor(cpuHandle);
	};
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
	ImGuizmo::BeginFrame();

	// ギズモはフレームごとにGizmo()で追加し直す.
	gizmoTargets_.clear();

	// Inspectorのタブはフレームごとに追加し直す.
	inspectorTabs_.clear();
	hasInspectorContentCursor_ = false;
	inspectorTabState_ = InspectorTabState::kNone;
#endif // USE_IMGUI
}

void ImGuiManager::EndFrame(ID3D12GraphicsCommandList* commandList) {
#ifdef USE_IMGUI
	// パーティクルのEmitterの調整(Inspectorのタブ. ギズモを使うのでGameウィンドウより先に呼ぶ).
	ParticleManager::GetInstance()->DrawEmitterGui();
	// GUIで設置するエミッター("エミッター"ウィンドウとInspectorのタブ).
	EmitterManager::GetInstance()->DrawGui();

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
	// ゲーム画面上のドラッグ(ギズモ / カメラ操作)でウィンドウが動かないようにする.
	ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove);
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

		// ゲーム画面の上にギズモを重ねる.
		DrawGizmos(gameViewPos_, gameViewSize_);
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

	// 同じ行の右端に デバッグカメラ の切り替えボタン.
	ImGui::SameLine();
	DrawDebugCameraButton();

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

/*=============================================================
Gizmo
=============================================================*/
void ImGuiManager::DrawGizmos(const Vector2& imagePos, const Vector2& imageSize) {
	isGizmoUsing_ = false;
	if (gizmoTargets_.empty()) {
		return;
	}

	// 操作の切り替え(停止中 / 一時停止中は W / E / R キーでも切り替えられる. 右ドラッグ中はカメラ操作なので切り替えない).
	if (playState_ != PlayState::kPlay && isGameViewHovered_ && !ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::GetIO().WantTextInput) {
		if (ImGui::IsKeyPressed(ImGuiKey_W, false)) { gizmoOperation_ = GizmoOperation::kTranslate; }
		if (ImGui::IsKeyPressed(ImGuiKey_E, false)) { gizmoOperation_ = GizmoOperation::kRotate; }
		if (ImGui::IsKeyPressed(ImGuiKey_R, false)) { gizmoOperation_ = GizmoOperation::kScale; }
	}

	// ゲーム画面の左上に 操作の種類 / ワールド・ローカル の切り替えボタンを重ねる.
	ImGui::SetCursorScreenPos(ImVec2(imagePos.x + 6.0f, imagePos.y + 6.0f));
	const ImVec4 activeColor = ImVec4(0.20f, 0.45f, 0.80f, 1.0f);
	auto operationButton = [this, &activeColor](const char* label, GizmoOperation operation, const char* tooltip) {
		const bool isActive = gizmoOperation_ == operation;
		if (isActive) { ImGui::PushStyleColor(ImGuiCol_Button, activeColor); }
		if (ImGui::Button(label)) {
			gizmoOperation_ = operation;
		}
		if (isActive) { ImGui::PopStyleColor(); }
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("%s", tooltip);
		}
		ImGui::SameLine();
	};
	operationButton("移動", GizmoOperation::kTranslate, "移動 (W)");
	operationButton("回転", GizmoOperation::kRotate, "回転 (E)");
	operationButton("拡縮", GizmoOperation::kScale, "拡大縮小 (R)");
	if (ImGui::Button(isGizmoWorld_ ? "ワールド" : "ローカル")) {
		isGizmoWorld_ = !isGizmoWorld_;
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("ギズモの向き(拡縮は常にローカル)");
	}

	// カメラの行列(ImGuizmoとエンジンは同じ並び(行ベクトル・DirectXの射影)なのでそのまま渡せる).
	Camera* camera = Camera::GetInstance();
	const Matrix4x4 view = camera->GetMatrix().Inverse();
	const Matrix4x4 projection = camera->GetProjectionMatrix();

	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(imagePos.x, imagePos.y, imageSize.x, imageSize.y);

	for (const GizmoTarget& target : gizmoTargets_) {
		// 許可されていない操作が選ばれていれば、そのギズモは出さない.
		uint32_t operationFlag = kGizmoTranslate;
		ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
		switch (gizmoOperation_) {
		case GizmoOperation::kRotate: operationFlag = kGizmoRotate; operation = ImGuizmo::ROTATE; break;
		case GizmoOperation::kScale:  operationFlag = kGizmoScale;  operation = ImGuizmo::SCALE;  break;
		default: break;
		}
		if ((target.operations & operationFlag) == 0) {
			continue;
		}

		Transform& transform = *target.transform;
		Matrix4x4 world = transform.GetAffineMatrix();
		ImGuizmo::PushID(target.id.c_str());
		const bool isChanged = ImGuizmo::Manipulate(&view.matrix[0][0], &projection.matrix[0][0], operation,
			isGizmoWorld_ ? ImGuizmo::WORLD : ImGuizmo::LOCAL, &world.matrix[0][0]);
		if (ImGuizmo::IsOver() || ImGuizmo::IsUsing()) {
			isGizmoUsing_ = true;
		}
		ImGuizmo::PopID();
		if (!isChanged) {
			continue;
		}

		// 親があれば親から見た行列に直す.
		Matrix4x4 local = world;
		if (const Transform* parent = transform.GetParent()) {
			local = world * parent->GetAffineMatrix().Inverse();
		}
		float translation[3];
		float rotation[3];
		float scale[3];
		ImGuizmo::DecomposeMatrixToComponents(&local.matrix[0][0], translation, rotation, scale);
		// 操作した成分だけ書き込む(他の成分に分解の誤差が乗らないように).
		switch (gizmoOperation_) {
		case GizmoOperation::kTranslate:
			transform.translate = { translation[0], translation[1], translation[2] };
			break;
		case GizmoOperation::kRotate: {
			const float degreeToRadian = 3.14159265f / 180.0f;
			transform.rotate = { rotation[0] * degreeToRadian, rotation[1] * degreeToRadian, rotation[2] * degreeToRadian };
			break;
		}
		case GizmoOperation::kScale:
			transform.scale = { scale[0], scale[1], scale[2] };
			break;
		}
	}
}

/*=============================================================
デバッグカメラ
=============================================================*/
void ImGuiManager::DrawDebugCameraButton() {
	Camera* camera = Camera::GetInstance();
	const bool isDebugCamera = camera->GetUseDebugCamera();
	std::string label = "デバッグカメラ" + static_cast<std::string>(isDebugCamera ? "\n起動中" : "\n停止中");
	const float buttonWidth = ImGui::CalcTextSize(label.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.0f;
	// 再生ボタンと同じ高さにして、ウィンドウの右端に寄せる.
	const float buttonHeight = kPlayControlIconSize_ + ImGui::GetStyle().FramePadding.y * 2.0f;
	const float rightX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonWidth - 8.0f;
	if (ImGui::GetCursorPosX() < rightX) {
		ImGui::SetCursorPosX(rightX);
	}
	if (isDebugCamera) { ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.45f, 0.80f, 1.0f)); }
	if (ImGui::Button(label.c_str(), ImVec2(buttonWidth, buttonHeight))) {
		camera->SetUseDebugCamera(!isDebugCamera);
	}
	if (isDebugCamera) { ImGui::PopStyleColor(); }
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Game画面上でホイールドラッグ: 視点を回す\nホイールドラッグ中に WASD / Space / Shift: 移動");
	}
}
#endif // USE_IMGUI

void ImGuiManager::Gizmo(const std::string& id, Transform& transform, uint32_t operations) {
#ifdef USE_IMGUI
	gizmoTargets_.push_back({ id, &transform, operations });
#else
	(void)id;
	(void)transform;
	(void)operations;
#endif // USE_IMGUI
}
