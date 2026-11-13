#include "ParticleSettingsManager.h"
#include <algorithm>
#include <cfloat>
#include <filesystem>
#include <fstream>
#include <format>
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "../Engine/Renderer/Camera.h"
#include "../Engine/Renderer/ParticleNodeMove.h"
#include "../externals/magic_enum/magic_enum.hpp"

#ifdef USE_IMGUI
#include "../externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace {
	nlohmann::json ToJsonArray(const Vector3& v) {
		return nlohmann::json::array({ v.x, v.y, v.z });
	}

	Vector3 FromJsonArray(const nlohmann::json& json, const Vector3& defaultValue) {
		if (!json.is_array() || json.size() != 3 || !json[0].is_number() || !json[1].is_number() || !json[2].is_number()) {
			return defaultValue;
		}
		return { json[0].get<float>(), json[1].get<float>(), json[2].get<float>() };
	}

	// enum を名前の文字列で保存する(順番を入れ替えても読み込めるように).
	template <typename T>
	std::string EnumToString(T value) {
		return std::string(magic_enum::enum_name(value));
	}

	template <typename T>
	T EnumFromJson(const nlohmann::json& json, const char* key, T defaultValue) {
		if (!json.contains(key) || !json[key].is_string()) {
			return defaultValue;
		}
		return magic_enum::enum_cast<T>(json[key].get<std::string>()).value_or(defaultValue);
	}

#ifdef USE_IMGUI
	// 表示名.
	const char* const kShapeNames[] = { "テクスチャ(板ポリ)", "モデル", "Primitive図形" };
	const char* const kPrimitiveNames[] = { "平面", "箱", "球", "トーラス", "リング", "円柱" };
	const char* const kBillboardNames[] = { "なし", "全方向", "X軸のみ(未対応)", "Y軸のみ(未対応)", "Z軸のみ(未対応)" };
	// Particles::Move の kNodeMove より前の動き(kNodeMoveはノードを選んだ時に使う).
	const char* const kMoveNames[] = { "通常", "炎", "斬撃", "爆発", "爆発(モノクロ)", "チャージ", "数字" };

	/// <summary>
	/// 名前の一覧から1つ選ぶコンボボックス(開いた時に絞り込みの入力欄を出す).
	/// </summary>
	bool NameCombo(const char* label, std::string& value, const std::vector<std::string>& names, char* filter, size_t filterSize) {
		bool isChanged = false;
		if (ImGui::BeginCombo(label, value.empty() ? "(選択してください)" : value.c_str(), ImGuiComboFlags_HeightLarge)) {
			if (ImGui::IsWindowAppearing()) {
				filter[0] = '\0';
				ImGui::SetKeyboardFocusHere();
			}
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputTextWithHint("##Filter", "絞り込み", filter, filterSize);
			for (const std::string& name : names) {
				if (filter[0] != '\0' && name.find(filter) == std::string::npos) {
					continue;
				}
				const bool isSelected = name == value;
				if (ImGui::Selectable(name.c_str(), isSelected)) {
					value = name;
					isChanged = true;
				}
				if (isSelected && ImGui::IsWindowAppearing()) {
					ImGui::SetScrollHereY();
				}
			}
			ImGui::EndCombo();
		}
		return isChanged;
	}

	// メッシュが1つのモデルの名前(パーティクルにできるもの).
	std::vector<std::string> GetParticleModelNames() {
		ModelManager* modelManager = ModelManager::GetInstance();
		std::vector<std::string> names;
		for (const std::string& name : modelManager->GetModelNames()) {
			if (modelManager->GetModelInfo(name).modelData.size() == 1) {
				names.push_back(name);
			}
		}
		return names;
	}
#endif // USE_IMGUI
}

ParticleSettingsManager* ParticleSettingsManager::GetInstance() {
	static ParticleSettingsManager instance;
	return &instance;
}

/*=============================================================
読み込み / 保存
=============================================================*/
void ParticleSettingsManager::Load() {
	if (isLoaded_) {
		return;
	}
	isLoaded_ = true;

	nlohmann::json json = nlohmann::json::object();
	std::ifstream ifs(kFilePath);
	if (ifs) {
		json = nlohmann::json::parse(ifs, nullptr, false);
		if (json.is_discarded()) {
			json = nlohmann::json::object();
			message_ = std::format("読み込みに失敗しました: {}", kFilePath);
		}
	}
	FromJson(json);
	savedJson_ = ToJson();

	// 読み込んだパーティクルをすぐ作る(シーンやエミッターから使えるように).
	ApplyChanges();
}

bool ParticleSettingsManager::Save() {
	std::error_code ec;
	std::filesystem::create_directories(kDirectoryPath, ec);
	std::ofstream ofs(kFilePath);
	if (!ofs) {
		message_ = std::format("保存に失敗しました: {}", kFilePath);
		return false;
	}
	const nlohmann::json json = ToJson();
	ofs << json.dump(4);
	savedJson_ = json;
	message_ = std::format("保存しました: {}", kFilePath);
	return true;
}

nlohmann::json ParticleSettingsManager::ToJson() const {
	nlohmann::json particlesJson = nlohmann::json::array();
	for (const Entry& entry : entries_) {
		particlesJson.push_back(SettingsToJson(entry.settings));
	}
	return { { "particles", std::move(particlesJson) } };
}

void ParticleSettingsManager::FromJson(const nlohmann::json& json) {
	entries_.clear();
	if (!json.is_object() || !json.contains("particles") || !json["particles"].is_array()) {
		return;
	}
	for (const nlohmann::json& particleJson : json["particles"]) {
		ParticleSettings settings = SettingsFromJson(particleJson);
		// 名前が無い / 被っているものは読み込まない.
		if (settings.name.empty() || FindEntry(settings.name)) {
			continue;
		}
		entries_.push_back({ std::move(settings) });
	}
}

nlohmann::json ParticleSettingsManager::SettingsToJson(const ParticleSettings& settings) {
	return {
		{ "name", settings.name },
		{ "shape", EnumToString(settings.shape) },
		{ "texture", settings.textureName },
		{ "model", settings.modelName },
		{ "primitive", EnumToString(settings.primitiveType) },
		{ "radiusA", settings.primitiveParam.radiusA },
		{ "radiusB", settings.primitiveParam.radiusB },
		{ "billboard", EnumToString(settings.billboardType) },
		{ "useNodeMove", settings.useNodeMove },
		{ "move", EnumToString(settings.moveType) },
		{ "nodeMove", settings.nodeMoveName },
		{ "size", ToJsonArray(settings.size) },
	};
}

ParticleSettings ParticleSettingsManager::SettingsFromJson(const nlohmann::json& json) {
	ParticleSettings settings;
	if (!json.is_object()) {
		return settings;
	}
	settings.name = json.value("name", std::string{});
	settings.shape = EnumFromJson(json, "shape", settings.shape);
	settings.textureName = json.value("texture", settings.textureName);
	settings.modelName = json.value("model", settings.modelName);
	settings.primitiveType = EnumFromJson(json, "primitive", settings.primitiveType);
	settings.primitiveParam.radiusA = json.value("radiusA", settings.primitiveParam.radiusA);
	settings.primitiveParam.radiusB = json.value("radiusB", settings.primitiveParam.radiusB);
	settings.billboardType = EnumFromJson(json, "billboard", settings.billboardType);
	settings.useNodeMove = json.value("useNodeMove", settings.useNodeMove);
	settings.moveType = EnumFromJson(json, "move", settings.moveType);
	settings.nodeMoveName = json.value("nodeMove", settings.nodeMoveName);
	if (json.contains("size")) {
		settings.size = FromJsonArray(json["size"], settings.size);
	}
	return settings;
}

/*=============================================================
追加 / 削除 / 反映
=============================================================*/
ParticleSettings* ParticleSettingsManager::Add(const std::string& name) {
	// "__" から始まる名前はエディター用なので使わない.
	if (name.empty() || name.rfind("__", 0) == 0) {
		message_ = "名前が空か、\"__\" から始まっています";
		return nullptr;
	}
	if (FindEntry(name) || ParticleManager::GetInstance()->HasParticles(name)) {
		message_ = std::format("\"{}\" のパーティクルは既にあります", name);
		return nullptr;
	}
	// 消したばかり(まだParticleManagerに残っている)の名前は、消えるまで待つ.
	if (std::find(removedNames_.begin(), removedNames_.end(), name) != removedNames_.end()) {
		message_ = std::format("\"{}\" は消している途中です. もう一度押してください", name);
		return nullptr;
	}

	Entry entry;
	entry.settings.name = name;
	// 板ポリが見えるようにカメラの方を向けておく.
	entry.settings.billboardType = BillboardType::kAllAxis;
	if (TextureManager::GetInstance()->HasTexture("effect_plane")) {
		entry.settings.textureName = "effect_plane";
	}
	entries_.push_back(std::move(entry));
	message_ = std::format("\"{}\" を追加しました", name);
	return &entries_.back().settings;
}

void ParticleSettingsManager::Remove(const std::string& name) {
	// name が消す設定の中の名前のこともあるので、消す前に写しておく.
	const std::string removeName = name;
	auto it = std::find_if(entries_.begin(), entries_.end(), [&](const Entry& entry) { return entry.settings.name == removeName; });
	if (it == entries_.end()) {
		return;
	}
	entries_.erase(it);
	removedNames_.push_back(removeName);
	message_ = std::format("\"{}\" を削除しました", removeName);
}

ParticleSettings* ParticleSettingsManager::Find(const std::string& name) {
	Entry* entry = FindEntry(name);
	return entry ? &entry->settings : nullptr;
}

ParticleSettingsManager::Entry* ParticleSettingsManager::FindEntry(const std::string& name) {
	for (Entry& entry : entries_) {
		if (entry.settings.name == name) {
			return &entry;
		}
	}
	return nullptr;
}

void ParticleSettingsManager::ApplyChanges() {
	ParticleManager* particleManager = ParticleManager::GetInstance();

	for (const std::string& name : removedNames_) {
		particleManager->RemoveParticles(name);
	}
	removedNames_.clear();

	for (Entry& entry : entries_) {
		if (!entry.needsApply) {
			continue;
		}
		particleManager->ApplyParticleSettings(entry.settings, entry.needsRebuild);
		entry.needsApply = false;
		entry.needsRebuild = false;
	}
}

/*=============================================================
GUI
=============================================================*/
void ParticleSettingsManager::DrawGui() {
#ifdef USE_IMGUI
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	const bool isDirty = ToJson() != savedJson_;

	// 保存していない変更があればタイトルに * を付ける(###以降をIDにしてウィンドウは同じにする).
	const std::string title = std::format("パーティクル{}###ParticleSettingsManager", isDirty ? " *" : "");
	ImGui::SetNextWindowSize(ImVec2(300.0f, 300.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(title.c_str())) {
		ImGui::SetNextItemWidth((std::max)(ImGui::GetContentRegionAvail().x - 120.0f, 60.0f));
		const bool isEnter = ImGui::InputText("##NewName", newNameBuffer_, sizeof(newNameBuffer_), ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::SameLine();
		if (ImGui::Button("追加") || isEnter) {
			if (ParticleSettings* settings = Add(newNameBuffer_)) {
				imguiManager->SelectInspectorTab(GetInspectorTabName(settings->name));
			}
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("入力した名前のパーティクルを作ります\n(コードの CreateNewParticles で作ったパーティクルと同じく、名前で使えます)");
		}
		ImGui::SameLine();
		if (ImGui::Button("保存")) {
			Save();
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("%s に保存します", kFilePath.c_str());
		}
		if (!message_.empty()) {
			ImGui::TextDisabled("%s", message_.c_str());
		}
		ImGui::Separator();

		// GUIで作ったパーティクルの一覧(クリックでInspectorのそのパーティクルのタブを開く).
		if (entries_.empty()) {
			ImGui::TextDisabled("パーティクルがありません(名前を入れて追加)");
		}
		for (const Entry& entry : entries_) {
			const std::string label = std::format("{}  ({})", entry.settings.name, kShapeNames[static_cast<int>(entry.settings.shape)]);
			if (ImGui::Selectable(label.c_str())) {
				imguiManager->SelectInspectorTab(GetInspectorTabName(entry.settings.name));
			}
		}

		// コードで作ったパーティクル(ここでは変えられない).
		if (ImGui::CollapsingHeader("コードで作ったパーティクル")) {
			for (const std::string& name : ParticleManager::GetInstance()->GetParticleNames()) {
				if (!FindEntry(name)) {
					ImGui::BulletText("%s", name.c_str());
				}
			}
		}
	}
	ImGui::End();

	// 各パーティクルのInspectorのタブ(削除ボタンで消えるので名前を先に集めておく).
	std::vector<std::string> names;
	for (const Entry& entry : entries_) {
		names.push_back(entry.settings.name);
	}
	for (const std::string& name : names) {
		if (Entry* entry = FindEntry(name)) {
			DrawInspector(*entry);
		}
	}
#endif // USE_IMGUI
}

void ParticleSettingsManager::DrawInspector(Entry& entry) {
#ifdef USE_IMGUI
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();
	if (!imguiManager->Begin(GetInspectorTabName(entry.settings.name))) {
		imguiManager->End();
		return;
	}

	ParticleSettings& settings = entry.settings;
	bool isRebuild = false; // 形 / テクスチャが変わった.
	bool isApply = false;   // ビルボード / 動き / 大きさが変わった.

	/*---------- CreateNewParticles ----------*/
	ImGui::SeparatorText("CreateNewParticles");

	// 形(どのオーバーロードにあたるか).
	int shape = static_cast<int>(settings.shape);
	if (ImGui::Combo("形", &shape, kShapeNames, IM_ARRAYSIZE(kShapeNames))) {
		settings.shape = static_cast<ParticleSettings::Shape>(shape);
		isRebuild = true;
	}

	if (settings.shape == ParticleSettings::Shape::kModel) {
		// モデル(メッシュが1つのものだけ).
		const std::vector<std::string> modelNames = GetParticleModelNames();
		isRebuild |= NameCombo("モデル", settings.modelName, modelNames, filterBuffer_, sizeof(filterBuffer_));
		if (std::find(modelNames.begin(), modelNames.end(), settings.modelName) == modelNames.end()) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "モデルを選んでください(今は板ポリで出ます)");
		}
	} else {
		// テクスチャ.
		TextureManager* textureManager = TextureManager::GetInstance();
		isRebuild |= NameCombo("テクスチャ", settings.textureName, textureManager->GetTextureNames(), filterBuffer_, sizeof(filterBuffer_));
		if (!textureManager->HasTexture(settings.textureName)) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "\"%s\" は登録されていません(白で出ます)", settings.textureName.c_str());
		}
	}

	if (settings.shape == ParticleSettings::Shape::kPrimitive) {
		int primitive = static_cast<int>(settings.primitiveType);
		if (ImGui::Combo("図形", &primitive, kPrimitiveNames, IM_ARRAYSIZE(kPrimitiveNames))) {
			settings.primitiveType = static_cast<PrimitiveType>(primitive);
			isRebuild = true;
		}
		// 半径などは トーラス / リング / 円柱 だけで使う.
		const char* labelA = nullptr;
		const char* labelB = nullptr;
		switch (settings.primitiveType) {
		case PrimitiveType::kTours: labelA = "半径"; labelB = "チューブの半径"; break;
		case PrimitiveType::kRing: labelA = "外側の半径"; labelB = "内側の半径"; break;
		case PrimitiveType::kCylinder: labelA = "上の半径"; labelB = "下の半径"; break;
		default: break;
		}
		if (labelA) {
			ImGui::DragFloat(labelA, &settings.primitiveParam.radiusA, 0.01f, 0.0f, 100.0f, "%.2f");
			isRebuild |= ImGui::IsItemDeactivatedAfterEdit();
			ImGui::DragFloat(labelB, &settings.primitiveParam.radiusB, 0.01f, 0.0f, 100.0f, "%.2f");
			isRebuild |= ImGui::IsItemDeactivatedAfterEdit();
		}
	}

	// ビルボード.
	int billboard = static_cast<int>(settings.billboardType);
	if (ImGui::Combo("ビルボード", &billboard, kBillboardNames, IM_ARRAYSIZE(kBillboardNames))) {
		settings.billboardType = static_cast<BillboardType>(billboard);
		isApply = true;
	}

	// 動き(Particles::Move / ノードで作った動き).
	int moveKind = settings.useNodeMove ? 1 : 0;
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted("動き");
	ImGui::SameLine();
	isApply |= ImGui::RadioButton("種類", &moveKind, 0);
	ImGui::SameLine();
	isApply |= ImGui::RadioButton("ノード", &moveKind, 1);
	settings.useNodeMove = moveKind == 1;
	if (settings.useNodeMove) {
		const std::string preview = settings.nodeMoveName.empty() ? "(選択してください)" : settings.nodeMoveName;
		if (ImGui::BeginCombo("ノードの動き", preview.c_str())) {
			// 開いた時にフォルダを見て一覧を作る.
			for (const std::string& name : ParticleNodeMove::GetSavedNames()) {
				if (ImGui::Selectable(name.c_str(), name == settings.nodeMoveName)) {
					settings.nodeMoveName = name;
					isApply = true;
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("パーティクルエディターで保存した動き(Resources/Node/Particle)");
		}
		if (settings.nodeMoveName.empty()) {
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "動きを選んでください(今は通常の動きです)");
		}
	} else {
		int move = (std::min)(static_cast<int>(settings.moveType), IM_ARRAYSIZE(kMoveNames) - 1);
		if (ImGui::Combo("動きの種類", &move, kMoveNames, IM_ARRAYSIZE(kMoveNames))) {
			settings.moveType = static_cast<Particles::Move>(move);
			isApply = true;
		}
	}

	/*---------- SetParticleSize ----------*/
	ImGui::SeparatorText("SetParticleSize");
	isApply |= ImGui::DragFloat3("大きさ", &settings.size.x, 0.01f, 0.0f, 100.0f, "%.2f");

	/*---------- 確認 / 削除 ----------*/
	ImGui::Separator();
	ParticleManager* particleManager = ParticleManager::GetInstance();
	const bool hasParticles = particleManager->HasParticles(settings.name) && !entry.needsApply;
	ImGui::BeginDisabled(!hasParticles);
	if (ImGui::Button("カメラの前に出す")) {
		// カメラの前(10先)に1つ出す.
		Matrix4x4 cameraMatrix = Camera::GetInstance()->GetMatrix();
		const Vector3 position = {
			cameraMatrix.matrix[3][0] + cameraMatrix.matrix[2][0] * 10.0f,
			cameraMatrix.matrix[3][1] + cameraMatrix.matrix[2][1] * 10.0f,
			cameraMatrix.matrix[3][2] + cameraMatrix.matrix[2][2] * 10.0f,
		};
		particleManager->SpawnParticles(settings.name, position);
	}
	ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
		ImGui::SetTooltip("見た目の確認用に、今見えているカメラの前に1つ出します(動くのは再生中だけです)");
	}
	if (hasParticles) {
		ImGui::SameLine();
		ImGui::TextDisabled("出ている数: %zu", particleManager->GetParticles(settings.name)->GetParticleCount());
	}

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.20f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.80f, 0.25f, 0.25f, 1.0f));
	const bool isDelete = ImGui::Button("削除");
	ImGui::PopStyleColor(2);
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("このパーティクルを消します(使っているエミッターは何も出さなくなります)");
	}

	imguiManager->End();

	// 変えた内容は次のフレームの最初に反映する.
	if (isRebuild || isApply) {
		entry.needsApply = true;
		entry.needsRebuild |= isRebuild;
	}
	if (isDelete) {
		Remove(settings.name);
	}
#else
	(void)entry;
#endif // USE_IMGUI
}
