#include "ParticleNodeEditor.h"
#include <algorithm>
#include <format>
#include "../Engine/Renderer/ParticleNodeMove.h"
#include "ParticleManager.h"
#include "TextureManager.h"
#include "ImGuiManager.h"
#include "../Engine/SystemFile/DeltaTime.h"

#ifdef USE_IMGUI
#include "../externals/imgui/imgui.h"
#endif // USE_IMGUI

ParticleNodeEditor* ParticleNodeEditor::GetInstance() {
	static ParticleNodeEditor instance;
	return &instance;
}

void ParticleNodeEditor::Initialize() {
	isInitialized_ = true;
	graph_.Initialize("ParticleNodeGraph");
	ParticleNodeMove::RegisterNodeTypes(graph_);
	previewMove_ = std::make_shared<ParticleNodeMove>();
	NewGraph();
}

void ParticleNodeEditor::Finalize() {
	graph_.Finalize();
	isInitialized_ = false;
}

void ParticleNodeEditor::NewGraph() {
	graph_.Clear();
	ParticleNodeMove::CreateDefaultNodes(graph_);
	graph_.RequestNavigateToContent();
	previewMove_->Compile(graph_);
	graph_.ConsumeChanged();
	isDirty_ = false;
}

bool ParticleNodeEditor::SaveGraph() {
	const std::string name = nameBuffer_;
	// ファイル名に使えない文字は保存しない.
	if (name.empty() || name.find_first_of("\\/:*?\"<>|") != std::string::npos) {
		message_ = "名前が空か、ファイル名に使えない文字(\\ / : * ? \" < > |)が入っています";
		return false;
	}
	const std::string path = ParticleNodeMove::GetFilePath(name);
	if (!graph_.Save(path)) {
		message_ = std::format("保存に失敗しました: {}", path);
		return false;
	}
	// 同じ動きを使っているパーティクルにすぐ反映する.
	ParticleNodeMove::Reload(name);
	isDirty_ = false;
	selectedLoadName_ = name;
	message_ = std::format("保存しました: {}", path);
	return true;
}

bool ParticleNodeEditor::LoadGraph(const std::string& name) {
	const std::string path = ParticleNodeMove::GetFilePath(name);
	if (!graph_.Load(path)) {
		message_ = std::format("読み込みに失敗しました: {}", path);
		return false;
	}
	strncpy_s(nameBuffer_, name.c_str(), _TRUNCATE);
	graph_.RequestNavigateToContent();
	previewMove_->Compile(graph_);
	graph_.ConsumeChanged();
	isDirty_ = false;
	message_ = std::format("読み込みました: {}", path);
	return true;
}

void ParticleNodeEditor::Draw() {
#ifdef USE_IMGUI
	if (!isInitialized_) {
		Initialize();
	}

	ImGui::SetNextWindowSize(ImVec2(1000.0f, 600.0f), ImGuiCond_FirstUseEver);
	// 保存していない変更があればタイトルに * を付ける(###以降をIDにしてウィンドウは同じにする).
	const std::string title = std::format("パーティクルエディター{}###ParticleNodeEditor", isDirty_ ? " *" : "");
	if (ImGui::Begin(title.c_str())) {
		DrawToolbar();
		DrawPreviewSettings();
		if (!message_.empty()) {
			ImGui::TextDisabled("%s", message_.c_str());
		}
		ImGui::Separator();
		graph_.DrawContent();
	}
	ImGui::End();

	// 編集したらプレビューの動きを作り直す.
	if (graph_.ConsumeChanged()) {
		previewMove_->Compile(graph_);
		isDirty_ = true;
	}

	UpdatePreview();
#endif // USE_IMGUI
}

void ParticleNodeEditor::DrawToolbar() {
#ifdef USE_IMGUI
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted("名前");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(160.0f);
	ImGui::InputText("##Name", nameBuffer_, sizeof(nameBuffer_));
	ImGui::SameLine();
	if (ImGui::Button("保存")) {
		SaveGraph();
	}

	ImGui::SameLine(0.0f, 24.0f);
	ImGui::SetNextItemWidth(160.0f);
	if (ImGui::BeginCombo("##LoadName", selectedLoadName_.empty() ? "保存した動き" : selectedLoadName_.c_str())) {
		// 開いた時にフォルダを見て一覧を作る.
		for (const std::string& name : ParticleNodeMove::GetSavedNames()) {
			if (ImGui::Selectable(name.c_str(), name == selectedLoadName_)) {
				selectedLoadName_ = name;
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(selectedLoadName_.empty());
	if (ImGui::Button("読み込み")) {
		LoadGraph(selectedLoadName_);
	}
	ImGui::EndDisabled();

	ImGui::SameLine(0.0f, 24.0f);
	if (ImGui::Button("新規")) {
		NewGraph();
		strncpy_s(nameBuffer_, "new_particle", _TRUNCATE);
		message_ = "新しい動きを作りました";
	}
	ImGui::SameLine();
	if (ImGui::Button("全体を表示")) {
		graph_.RequestNavigateToContent();
	}
#endif // USE_IMGUI
}

void ParticleNodeEditor::DrawPreviewSettings() {
#ifdef USE_IMGUI
	ImGui::Checkbox("プレビュー", &isPreviewEnabled_);
	ImGui::BeginDisabled(!isPreviewEnabled_);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(170.0f);
	ImGui::DragFloat3("位置", &previewPosition_.x, 0.05f);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.0f);
	ImGui::DragInt("個数", &previewCount_, 0.2f, 1, 100);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.0f);
	ImGui::DragFloat("間隔(秒)", &previewInterval_, 0.005f, 0.01f, 5.0f, "%.2f");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60.0f);
	ImGui::DragFloat("大きさ", &previewSize_, 0.01f, 0.01f, 10.0f, "%.2f");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(110.0f);
	ImGui::InputText("テクスチャ", previewTexture_, sizeof(previewTexture_));
	ImGui::EndDisabled();
	if (isPreviewEnabled_ && !ImGuiManager::GetInstance()->IsPlaying()) {
		ImGui::SameLine();
		ImGui::TextDisabled("(再生中にプレビューされます)");
	}
#endif // USE_IMGUI
}

void ParticleNodeEditor::UpdatePreview() {
	ParticleManager* particleManager = ParticleManager::GetInstance();
	if (!isPreviewEnabled_ || !ImGuiManager::GetInstance()->IsPlaying()) {
		previewTimer_ = 0.0f;
		return;
	}

	TextureManager* textureManager = TextureManager::GetInstance();
	const std::string textureName = textureManager->HasTexture(previewTexture_) ? std::string(previewTexture_) : std::string("white_template");

	// テクスチャが変わった / シーンの読み込みで消えた時は作り直す.
	if (particleManager->HasParticles(kPreviewParticleName) && appliedPreviewTexture_ != textureName) {
		particleManager->RemoveParticles(kPreviewParticleName);
	}
	if (!particleManager->HasParticles(kPreviewParticleName)) {
		particleManager->CreateNewParticles(kPreviewParticleName, textureManager->GetTextureInfo(textureName));
		Particles* particles = particleManager->GetParticles(kPreviewParticleName);
		particles->SetBillboardType(BillboardType::kAllAxis);
		particles->SetNodeMove(previewMove_);
		appliedPreviewTexture_ = textureName;
	}
	particleManager->SetParticleSize(kPreviewParticleName, { previewSize_, previewSize_, previewSize_ });

	previewTimer_ += DeltaTime::GetInstance()->GetDeltaTime();
	const float interval = (std::max)(previewInterval_, 0.01f);
	if (previewTimer_ >= interval) {
		previewTimer_ = std::fmod(previewTimer_, interval);
		for (int i = 0; i < previewCount_; ++i) {
			particleManager->SpawnParticles(kPreviewParticleName, previewPosition_);
		}
	}
}
