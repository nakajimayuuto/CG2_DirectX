#include "EmitterManager.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <format>
#include "ParticleManager.h"
#include "ImGuiManager.h"
#include "../Engine/Renderer/Camera.h"

#ifdef USE_IMGUI
#include "../externals/imgui/imgui.h"
#endif // USE_IMGUI

EmitterManager* EmitterManager::GetInstance() {
	static EmitterManager instance;
	return &instance;
}

/*=============================================================
読み込み / 保存
=============================================================*/
void EmitterManager::LoadScene(const std::string& sceneName) {
	// 今のシーンの編集中の内容を残しておく(終了ボタン / シーンの切り替えで作り直しても消えないように).
	if (!sceneName_.empty()) {
		sceneStates_[sceneName_] = { ToJson(), savedJson_ };
	}

	sceneName_ = sceneName;
	emitters_.clear();
	message_.clear();

	// 一度開いたシーンなら編集中の内容に戻す.
	auto it = sceneStates_.find(sceneName_);
	if (it != sceneStates_.end()) {
		FromJson(it->second.current);
		savedJson_ = it->second.saved;
		return;
	}

	// 初めて開くシーンはファイルから読み込む.
	nlohmann::json json = nlohmann::json::object();
	std::ifstream ifs(GetFilePath());
	if (ifs) {
		json = nlohmann::json::parse(ifs, nullptr, false);
		if (json.is_discarded()) {
			json = nlohmann::json::object();
			message_ = std::format("読み込みに失敗しました: {}", GetFilePath());
		}
	}
	FromJson(json);
	savedJson_ = ToJson();
}

bool EmitterManager::Save() {
	std::error_code ec;
	std::filesystem::create_directories(kDirectoryPath, ec);
	std::ofstream ofs(GetFilePath());
	if (!ofs) {
		message_ = std::format("保存に失敗しました: {}", GetFilePath());
		return false;
	}
	const nlohmann::json json = ToJson();
	ofs << json.dump(4);
	savedJson_ = json;
	message_ = std::format("保存しました: {}", GetFilePath());
	return true;
}

nlohmann::json EmitterManager::ToJson() const {
	nlohmann::json emittersJson = nlohmann::json::array();
	for (const auto& emitter : emitters_) {
		emittersJson.push_back(emitter->ToJson());
	}
	return { { "emitters", std::move(emittersJson) } };
}

void EmitterManager::FromJson(const nlohmann::json& json) {
	emitters_.clear();
	if (!json.is_object() || !json.contains("emitters") || !json["emitters"].is_array()) {
		return;
	}
	for (const nlohmann::json& emitterJson : json["emitters"]) {
		auto emitter = std::make_unique<ParticleEmitterEntity>();
		emitter->Initialize();
		emitter->SetInspectorName(MakeUniqueName());
		emitter->FromJson(emitterJson);
		// 名前が被っていたら付け直す(Inspectorのタブが同じになってしまうため).
		if (FindEmitter(emitter->GetInspectorName())) {
			emitter->SetInspectorName(MakeUniqueName());
		}
		emitters_.push_back(std::move(emitter));
	}
}

/*=============================================================
エミッター
=============================================================*/
ParticleEmitterEntity* EmitterManager::AddEmitter(const std::string& particleName, const Vector3& position) {
	auto emitter = std::make_unique<ParticleEmitterEntity>();
	emitter->Initialize();
	emitter->SetInspectorName(MakeUniqueName());
	emitter->SetParticleName(particleName);
	emitter->GetEntityTransform().translate = position;
	emitters_.push_back(std::move(emitter));
	return emitters_.back().get();
}

ParticleEmitterEntity* EmitterManager::FindEmitter(const std::string& name) const {
	for (const auto& emitter : emitters_) {
		if (emitter->GetInspectorName() == name) {
			return emitter.get();
		}
	}
	return nullptr;
}

std::string EmitterManager::MakeUniqueName() const {
	for (int number = 1;; ++number) {
		std::string name = std::format("Emitter_{}", number);
		if (!FindEmitter(name)) {
			return name;
		}
	}
}

void EmitterManager::Update() {
	for (auto& emitter : emitters_) {
		emitter->Update();
	}
}

void EmitterManager::Draw() {
	for (auto& emitter : emitters_) {
		emitter->Draw();
	}
}

/*=============================================================
GUI
=============================================================*/
void EmitterManager::DrawGui() {
#ifdef USE_IMGUI
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	// 前のフレームでInspectorの削除ボタンが押されたエミッターを消す.
	// (押されたフレームにはギズモにTransformを渡しているので、そのフレームでは消さない).
	std::erase_if(emitters_, [](const auto& emitter) { return emitter->IsDeleteRequested(); });

	const bool isDirty = ToJson() != savedJson_;

	// 保存していない変更があればタイトルに * を付ける(###以降をIDにしてウィンドウは同じにする).
	const std::string title = std::format("エミッター{}###EmitterManager", isDirty ? " *" : "");
	ImGui::SetNextWindowSize(ImVec2(300.0f, 300.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(title.c_str())) {
		ImGui::TextDisabled("シーン: %s", sceneName_.c_str());

		if (ImGui::Button("追加")) {
			// カメラの前(10先)に置き、パーティクルは最初に作られているものにする.
			Matrix4x4 cameraMatrix = Camera::GetInstance()->GetMatrix();
			const Vector3 position = {
				cameraMatrix.matrix[3][0] + cameraMatrix.matrix[2][0] * 10.0f,
				cameraMatrix.matrix[3][1] + cameraMatrix.matrix[2][1] * 10.0f,
				cameraMatrix.matrix[3][2] + cameraMatrix.matrix[2][2] * 10.0f,
			};
			const std::vector<std::string> particleNames = ParticleManager::GetInstance()->GetParticleNames();
			ParticleEmitterEntity* emitter = AddEmitter(particleNames.empty() ? std::string{} : particleNames.front(), position);
			imguiManager->SelectInspectorTab(emitter->GetInspectorName());
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("今見えているカメラの前にエミッターを置きます\n停止中のカメラは再生中のカメラと位置が違うことがあるので、\n再生中 / 一時停止中に置くとゲームの画面に映る位置に置けます");
		}
		ImGui::SameLine();
		if (ImGui::Button("保存")) {
			Save();
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("%s に保存します", GetFilePath().c_str());
		}
		if (!message_.empty()) {
			ImGui::TextDisabled("%s", message_.c_str());
		}
		ImGui::Separator();

		// 一覧(クリックでInspectorのそのエミッターのタブを開く).
		if (emitters_.empty()) {
			ImGui::TextDisabled("エミッターがありません(追加で設置)");
		}
		for (const auto& emitter : emitters_) {
			const std::string label = std::format("{}  ({})", emitter->GetInspectorName(),
				emitter->GetParticleName().empty() ? "パーティクル未設定" : emitter->GetParticleName());
			if (ImGui::Selectable(label.c_str())) {
				imguiManager->SelectInspectorTab(emitter->GetInspectorName());
			}
		}
	}
	ImGui::End();

	// 各エミッターのInspectorのタブ.
	for (auto& emitter : emitters_) {
		emitter->InspectorUpdate();
	}
#endif // USE_IMGUI
}
