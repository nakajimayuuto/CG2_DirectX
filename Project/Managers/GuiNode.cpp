#include "GuiNode.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <cfloat>
#include "../externals/nlohmann/json.hpp"

#ifdef USE_IMGUI
#include "../externals/imgui/imgui.h"
#include "../externals/imgui_node/imgui_node_editor.h"
namespace ed = ax::NodeEditor;
#endif // USE_IMGUI

namespace {
using json = nlohmann::json;

// ピンの値 → JSON.
json PinValueToJson(const GuiNode::PinValue& value) {
	return std::visit([](const auto& v) -> json {
		using T = std::decay_t<decltype(v)>;
		if constexpr (std::is_same_v<T, std::monostate>) {
			return nullptr;
		} else if constexpr (std::is_same_v<T, Vector2>) {
			return json::array({ v.x, v.y });
		} else if constexpr (std::is_same_v<T, Vector3>) {
			return json::array({ v.x, v.y, v.z });
		} else if constexpr (std::is_same_v<T, Vector4>) {
			return json::array({ v.x, v.y, v.z, v.w });
		} else {
			return v;
		}
	}, value);
}

// JSON → ピンの値(型が合わなければ既定値).
GuiNode::PinValue PinValueFromJson(GuiNode::PinType type, const json& j) {
	using PinType = GuiNode::PinType;
	// 配列の要素数を確認してfloatで読む.
	auto readFloats = [&j](size_t count, float* out) {
		if (!j.is_array() || j.size() != count) { return false; }
		for (size_t i = 0; i < count; ++i) {
			if (!j[i].is_number()) { return false; }
			out[i] = j[i].get<float>();
		}
		return true;
	};

	switch (type) {
	case PinType::kBool:
		if (j.is_boolean()) { return j.get<bool>(); }
		break;
	case PinType::kInt:
		if (j.is_number()) { return j.get<int>(); }
		break;
	case PinType::kFloat:
		if (j.is_number()) { return j.get<float>(); }
		break;
	case PinType::kVector2: {
		Vector2 v{};
		if (readFloats(2, &v.x)) { return v; }
		break;
	}
	case PinType::kVector3: {
		Vector3 v{};
		if (readFloats(3, &v.x)) { return v; }
		break;
	}
	case PinType::kColor: {
		Vector4 v{};
		if (readFloats(4, &v.x)) { return v; }
		break;
	}
	case PinType::kString:
		if (j.is_string()) { return j.get<std::string>(); }
		break;
	default:
		break;
	}
	return GuiNode::GetDefaultValue(type);
}

bool IsUnsignedId(const json& j, const char* key) {
	return j.is_object() && j.contains(key) && j[key].is_number_unsigned();
}

#ifdef USE_IMGUI
// ピンの型ごとの色.
ImColor GetPinColor(GuiNode::PinType type) {
	switch (type) {
	case GuiNode::PinType::kFlow:    return ImColor(255, 255, 255);
	case GuiNode::PinType::kBool:    return ImColor(220, 48, 48);
	case GuiNode::PinType::kInt:     return ImColor(68, 201, 156);
	case GuiNode::PinType::kFloat:   return ImColor(147, 226, 74);
	case GuiNode::PinType::kVector2: return ImColor(124, 180, 255);
	case GuiNode::PinType::kVector3: return ImColor(255, 206, 64);
	case GuiNode::PinType::kColor:   return ImColor(218, 80, 200);
	case GuiNode::PinType::kString:  return ImColor(180, 120, 255);
	}
	return ImColor(255, 255, 255);
}

ImVec4 ToImVec4(const Vector4& v) { return ImVec4(v.x, v.y, v.z, v.w); }

// ポップアップの名前.
const char* const kNodeMenuPopup = "##GuiNodeNodeMenu";
const char* const kLinkMenuPopup = "##GuiNodeLinkMenu";
const char* const kPinMenuPopup = "##GuiNodePinMenu";
const char* const kCreateMenuPopup = "##GuiNodeCreateMenu";
const char* const kColorPickerPopup = "##GuiNodeColorPicker";
#endif // USE_IMGUI
} // namespace

GuiNode::~GuiNode() {
	Finalize();
}

void GuiNode::Initialize(const std::string& name) {
	Finalize();
	name_ = name;
#ifdef USE_IMGUI
	ed::Config config;
	// ノードの位置などはこのクラスで管理する(Save/Load)ので、ライブラリの設定ファイルは作らない.
	config.SettingsFile = nullptr;
	context_ = ed::CreateEditor(&config);
	// 全てのノードの位置を次の描画でエディターに反映し直す.
	for (auto& node : nodes_) {
		node->isPositionDirty_ = true;
	}
#endif // USE_IMGUI
}

void GuiNode::Finalize() {
#ifdef USE_IMGUI
	if (context_) {
		ed::DestroyEditor(context_);
		context_ = nullptr;
	}
#endif // USE_IMGUI
}

void GuiNode::RegisterNodeType(const NodeTypeDesc& desc) {
	nodeTypes_[desc.name] = desc;
}

/*=============================================================
グラフの操作
=============================================================*/
GuiNode::Node* GuiNode::AddNode(const std::string& typeName, const Vector2& position) {
	auto it = nodeTypes_.find(typeName);
	if (it == nodeTypes_.end()) {
		return nullptr;
	}
	auto node = std::make_unique<Node>();
	node->id = GenerateId();
	node->typeName = typeName;
	node->title = typeName;
	node->position = position;
	BuildPins(*node, it->second);

	nodes_.push_back(std::move(node));
	isChanged_ = true;
	return nodes_.back().get();
}

void GuiNode::BuildPins(Node& node, const NodeTypeDesc& desc) {
	auto build = [&](const std::vector<PinDesc>& pinDescs, PinKind kind, std::vector<Pin>& outPins) {
		outPins.clear();
		for (const PinDesc& pinDesc : pinDescs) {
			Pin pin;
			pin.id = GenerateId();
			pin.nodeId = node.id;
			pin.name = pinDesc.name;
			pin.kind = kind;
			pin.type = pinDesc.type;
			pin.showValue = pinDesc.showValue;
			// 初期値が型と合っていなければ型の既定値にする.
			PinValue defaultValue = GetDefaultValue(pin.type);
			pin.value = (pinDesc.defaultValue.index() == defaultValue.index()) ? pinDesc.defaultValue : defaultValue;
			outPins.push_back(std::move(pin));
		}
	};
	build(desc.inputs, PinKind::kInput, node.inputs);
	build(desc.outputs, PinKind::kOutput, node.outputs);
}

void GuiNode::BuildSubPins(Pin& pin) {
	const int count = GetComponentCount(pin.type);
	pin.subPins.clear();
	for (int i = 0; i < count; ++i) {
		Pin sub;
		sub.id = GenerateId();
		sub.nodeId = pin.nodeId;
		sub.name = pin.name + "." + GetComponentName(pin.type, i);
		sub.kind = pin.kind;
		sub.type = PinType::kFloat;
		sub.value = GetComponent(pin.value, i);
		sub.showValue = pin.showValue;
		sub.parentPinId = pin.id;
		sub.componentIndex = i;
		pin.subPins.push_back(std::move(sub));
	}
	pin.isSplit = count > 0;
}

void GuiNode::RemoveLinksOfPin(uint32_t pinId) {
	const size_t removed = std::erase_if(links_, [pinId](const Link& link) { return link.startPinId == pinId || link.endPinId == pinId; });
	if (removed > 0) {
		isChanged_ = true;
	}
}

void GuiNode::RemoveNode(uint32_t nodeId) {
	auto it = std::find_if(nodes_.begin(), nodes_.end(), [nodeId](const auto& node) { return node->id == nodeId; });
	if (it == nodes_.end()) {
		return;
	}
	// このノードのピンに繋がっているリンクも消す.
	std::erase_if(links_, [this, nodeId](const Link& link) {
		const Pin* start = FindPin(link.startPinId);
		const Pin* end = FindPin(link.endPinId);
		return (start && start->nodeId == nodeId) || (end && end->nodeId == nodeId);
	});
	nodes_.erase(it);
	isChanged_ = true;
}

uint32_t GuiNode::AddLink(uint32_t pinIdA, uint32_t pinIdB) {
	Pin* pinA = FindPin(pinIdA);
	Pin* pinB = FindPin(pinIdB);
	if (!pinA || !pinB || !CanLink(*pinA, *pinB)) {
		return 0;
	}
	const Pin* output = (pinA->kind == PinKind::kOutput) ? pinA : pinB;
	const Pin* input = (pinA->kind == PinKind::kOutput) ? pinB : pinA;

	// 入力ピンに繋がるリンクは1本だけ(前のリンクは置き換える).
	std::erase_if(links_, [input](const Link& link) { return link.endPinId == input->id; });

	Link link;
	link.id = GenerateId();
	link.startPinId = output->id;
	link.endPinId = input->id;
	links_.push_back(link);
	isChanged_ = true;
	return link.id;
}

void GuiNode::RemoveLink(uint32_t linkId) {
	const size_t removed = std::erase_if(links_, [linkId](const Link& link) { return link.id == linkId; });
	if (removed > 0) {
		isChanged_ = true;
	}
}

bool GuiNode::CanLink(const Pin& a, const Pin& b) const {
	return a.nodeId != b.nodeId && a.kind != b.kind && a.type == b.type && !a.isSplit && !b.isSplit;
}

GuiNode::Pin* GuiNode::SplitPin(uint32_t pinId) {
	Pin* pin = FindPin(pinId);
	if (!pin || pin->parentPinId != 0 || pin->isSplit || !CanSplit(pin->type)) {
		return nullptr;
	}
	RemoveLinksOfPin(pin->id);
	BuildSubPins(*pin);
	isChanged_ = true;
	return pin;
}

bool GuiNode::UnsplitPin(uint32_t pinId) {
	Pin* pin = FindPin(pinId);
	if (pin && pin->parentPinId != 0) {
		pin = FindPin(pin->parentPinId);
	}
	if (!pin || !pin->isSplit) {
		return false;
	}
	for (const Pin& sub : pin->subPins) {
		// 要素ピンで編集していた値を元のピンに戻す.
		if (const float* value = std::get_if<float>(&sub.value)) {
			SetComponent(pin->value, sub.componentIndex, *value);
		}
		RemoveLinksOfPin(sub.id);
	}
	pin->subPins.clear();
	pin->isSplit = false;
	isChanged_ = true;
	return true;
}

void GuiNode::SetNodePosition(uint32_t nodeId, const Vector2& position) {
	if (Node* node = FindNode(nodeId)) {
		node->position = position;
		node->isPositionDirty_ = true;
		isChanged_ = true;
	}
}

void GuiNode::Clear() {
	nodes_.clear();
	links_.clear();
	isChanged_ = true;
}

/*=============================================================
グラフの取得
=============================================================*/
GuiNode::Node* GuiNode::FindNode(uint32_t nodeId) const {
	for (const auto& node : nodes_) {
		if (node->id == nodeId) {
			return node.get();
		}
	}
	return nullptr;
}

GuiNode::Pin* GuiNode::FindPin(uint32_t pinId) const {
	if (pinId == 0) {
		return nullptr;
	}
	auto findIn = [pinId](std::vector<Pin>& pins) -> Pin* {
		for (Pin& pin : pins) {
			if (pin.id == pinId) { return &pin; }
			for (Pin& sub : pin.subPins) {
				if (sub.id == pinId) { return &sub; }
			}
		}
		return nullptr;
	};
	for (const auto& node : nodes_) {
		if (Pin* pin = findIn(node->inputs)) { return pin; }
		if (Pin* pin = findIn(node->outputs)) { return pin; }
	}
	return nullptr;
}

const GuiNode::Link* GuiNode::FindLink(uint32_t linkId) const {
	for (const Link& link : links_) {
		if (link.id == linkId) {
			return &link;
		}
	}
	return nullptr;
}

bool GuiNode::IsPinLinked(uint32_t pinId) const {
	return std::any_of(links_.begin(), links_.end(), [pinId](const Link& link) {
		return link.startPinId == pinId || link.endPinId == pinId;
	});
}

GuiNode::Pin* GuiNode::GetLinkedOutputPin(uint32_t inputPinId) const {
	for (const Link& link : links_) {
		if (link.endPinId == inputPinId) {
			return FindPin(link.startPinId);
		}
	}
	return nullptr;
}

std::vector<GuiNode::Pin*> GuiNode::GetLinkedInputPins(uint32_t outputPinId) const {
	std::vector<Pin*> result;
	for (const Link& link : links_) {
		if (link.startPinId == outputPinId) {
			if (Pin* pin = FindPin(link.endPinId)) {
				result.push_back(pin);
			}
		}
	}
	return result;
}

GuiNode::PinValue GuiNode::GetInputValue(const Pin& inputPin) const {
	// 分割されていれば要素ピンの値をまとめる.
	if (inputPin.isSplit) {
		PinValue value = inputPin.value;
		for (const Pin& sub : inputPin.subPins) {
			SetComponent(value, sub.componentIndex, GetInputValueAs<float>(sub));
		}
		return value;
	}
	if (const Pin* output = GetLinkedOutputPin(inputPin.id)) {
		// 出力側が分割されていれば、元のピンの値の要素.
		if (output->parentPinId != 0) {
			const Pin* parent = FindPin(output->parentPinId);
			return parent ? PinValue(GetComponent(parent->value, output->componentIndex)) : PinValue(0.0f);
		}
		return output->value;
	}
	return inputPin.value;
}

std::vector<uint32_t> GuiNode::GetSelectedNodeIds() const {
	std::vector<uint32_t> result;
#ifdef USE_IMGUI
	if (!context_) {
		return result;
	}
	ed::EditorContext* lastContext = ed::GetCurrentEditor();
	ed::SetCurrentEditor(context_);
	const int count = ed::GetSelectedObjectCount();
	if (count > 0) {
		std::vector<ed::NodeId> ids(count);
		const int nodeCount = ed::GetSelectedNodes(ids.data(), count);
		for (int i = 0; i < nodeCount; ++i) {
			result.push_back(static_cast<uint32_t>(ids[i].Get()));
		}
	}
	ed::SetCurrentEditor(lastContext);
#endif // USE_IMGUI
	return result;
}

bool GuiNode::ConsumeChanged() {
	const bool isChanged = isChanged_;
	isChanged_ = false;
	return isChanged;
}

/*=============================================================
値のユーティリティ
=============================================================*/
GuiNode::PinValue GuiNode::GetDefaultValue(PinType type) {
	switch (type) {
	case PinType::kBool:    return false;
	case PinType::kInt:     return 0;
	case PinType::kFloat:   return 0.0f;
	case PinType::kVector2: return Vector2{ 0.0f, 0.0f };
	case PinType::kVector3: return Vector3{ 0.0f, 0.0f, 0.0f };
	case PinType::kColor:   return Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
	case PinType::kString:  return std::string{};
	default:                return std::monostate{};
	}
}

int GuiNode::GetComponentCount(PinType type) {
	switch (type) {
	case PinType::kVector2: return 2;
	case PinType::kVector3: return 3;
	case PinType::kColor:   return 4;
	default:                return 0;
	}
}

const char* GuiNode::GetComponentName(PinType type, int index) {
	static const char* const kXyzw[] = { "X", "Y", "Z", "W" };
	static const char* const kRgba[] = { "R", "G", "B", "A" };
	if (index < 0 || index >= 4) {
		return "";
	}
	return (type == PinType::kColor) ? kRgba[index] : kXyzw[index];
}

float GuiNode::GetComponent(const PinValue& value, int index) {
	if (index < 0 || index >= 4) {
		return 0.0f;
	}
	if (const Vector2* v = std::get_if<Vector2>(&value)) {
		return index < 2 ? (&v->x)[index] : 0.0f;
	}
	if (const Vector3* v = std::get_if<Vector3>(&value)) {
		return index < 3 ? (&v->x)[index] : 0.0f;
	}
	if (const Vector4* v = std::get_if<Vector4>(&value)) {
		return (&v->x)[index];
	}
	return 0.0f;
}

void GuiNode::SetComponent(PinValue& value, int index, float component) {
	if (index < 0 || index >= 4) {
		return;
	}
	if (Vector2* v = std::get_if<Vector2>(&value)) {
		if (index < 2) { (&v->x)[index] = component; }
	} else if (Vector3* v3 = std::get_if<Vector3>(&value)) {
		if (index < 3) { (&v3->x)[index] = component; }
	} else if (Vector4* v4 = std::get_if<Vector4>(&value)) {
		(&v4->x)[index] = component;
	}
}

/*=============================================================
保存 / 読み込み
=============================================================*/
bool GuiNode::Save(const std::string& filePath) const {
	json root;
	root["nextId"] = nextId_;

	json nodesJson = json::array();
	for (const auto& node : nodes_) {
		json nodeJson;
		nodeJson["id"] = node->id;
		nodeJson["type"] = node->typeName;
		nodeJson["title"] = node->title;
		nodeJson["position"] = json::array({ node->position.x, node->position.y });
		nodeJson["userData"] = node->userData;
		auto pinsToJson = [](const std::vector<Pin>& pins) {
			json pinsJson = json::array();
			for (const Pin& pin : pins) {
				json pinJson = { { "id", pin.id }, { "name", pin.name }, { "value", PinValueToJson(pin.value) } };
				if (pin.isSplit) {
					pinJson["split"] = true;
					json subsJson = json::array();
					for (const Pin& sub : pin.subPins) {
						subsJson.push_back({ { "id", sub.id }, { "value", PinValueToJson(sub.value) } });
					}
					pinJson["subPins"] = std::move(subsJson);
				}
				pinsJson.push_back(std::move(pinJson));
			}
			return pinsJson;
		};
		nodeJson["inputs"] = pinsToJson(node->inputs);
		nodeJson["outputs"] = pinsToJson(node->outputs);
		nodesJson.push_back(std::move(nodeJson));
	}
	root["nodes"] = std::move(nodesJson);

	json linksJson = json::array();
	for (const Link& link : links_) {
		linksJson.push_back({ { "id", link.id }, { "start", link.startPinId }, { "end", link.endPinId } });
	}
	root["links"] = std::move(linksJson);

	const std::filesystem::path path(filePath);
	if (path.has_parent_path()) {
		std::error_code ec;
		std::filesystem::create_directories(path.parent_path(), ec);
	}
	std::ofstream ofs(path);
	if (!ofs) {
		return false;
	}
	ofs << root.dump(4);
	return static_cast<bool>(ofs);
}

bool GuiNode::Load(const std::string& filePath) {
	std::ifstream ifs{ std::filesystem::path(filePath) };
	if (!ifs) {
		return false;
	}
	const json root = json::parse(ifs, nullptr, false);
	if (root.is_discarded() || !root.is_object()) {
		return false;
	}

	nodes_.clear();
	links_.clear();

	// ファイルにあるIDと被らないよう、新しく作るIDはファイルの最大値より後から振る.
	uint32_t maxId = 0;
	auto updateMaxId = [&maxId](const json& j) {
		if (IsUnsignedId(j, "id")) {
			maxId = (std::max)(maxId, j["id"].get<uint32_t>());
		}
	};
	const json emptyArray = json::array();
	const json& nodesJson = (root.contains("nodes") && root["nodes"].is_array()) ? root["nodes"] : emptyArray;
	const json& linksJson = (root.contains("links") && root["links"].is_array()) ? root["links"] : emptyArray;
	for (const json& nodeJson : nodesJson) {
		updateMaxId(nodeJson);
		for (const char* key : { "inputs", "outputs" }) {
			if (!nodeJson.contains(key) || !nodeJson[key].is_array()) { continue; }
			for (const json& pinJson : nodeJson[key]) {
				updateMaxId(pinJson);
				if (pinJson.contains("subPins") && pinJson["subPins"].is_array()) {
					for (const json& subJson : pinJson["subPins"]) { updateMaxId(subJson); }
				}
			}
		}
	}
	for (const json& linkJson : linksJson) { updateMaxId(linkJson); }
	nextId_ = maxId + 1;
	if (root.contains("nextId") && root["nextId"].is_number_unsigned()) {
		nextId_ = (std::max)(nextId_, root["nextId"].get<uint32_t>());
	}

	for (const json& nodeJson : nodesJson) {
		if (!nodeJson.contains("type") || !nodeJson["type"].is_string() || !IsUnsignedId(nodeJson, "id")) {
			continue;
		}
		auto typeIt = nodeTypes_.find(nodeJson["type"].get<std::string>());
		if (typeIt == nodeTypes_.end()) {
			// 登録されていない種類は読み飛ばす(繋がっていたリンクも後で読み飛ばされる).
			continue;
		}

		auto node = std::make_unique<Node>();
		node->id = nodeJson["id"].get<uint32_t>();
		node->typeName = typeIt->first;
		node->title = nodeJson.value("title", typeIt->first);
		node->userData = nodeJson.value("userData", std::string{});
		Vector2 position{ 0.0f, 0.0f };
		if (nodeJson.contains("position")) {
			const json& p = nodeJson["position"];
			if (p.is_array() && p.size() == 2 && p[0].is_number() && p[1].is_number()) {
				position = Vector2{ p[0].get<float>(), p[1].get<float>() };
			}
		}
		node->position = position;
		// ピンは今の種類の構成で作り、保存されていたピンと名前が同じものはIDと値(と分割)を引き継ぐ.
		BuildPins(*node, typeIt->second);
		auto restorePins = [this](std::vector<Pin>& pins, const json& pinsJson) {
			if (!pinsJson.is_array()) { return; }
			for (Pin& pin : pins) {
				for (const json& pinJson : pinsJson) {
					if (pinJson.value("name", std::string{}) != pin.name || !IsUnsignedId(pinJson, "id")) {
						continue;
					}
					pin.id = pinJson["id"].get<uint32_t>();
					if (pinJson.contains("value")) {
						pin.value = PinValueFromJson(pin.type, pinJson["value"]);
					}
					if (pinJson.value("split", false) && CanSplit(pin.type)) {
						BuildSubPins(pin);
						const json& subsJson = pinJson.contains("subPins") ? pinJson["subPins"] : json::array();
						for (size_t i = 0; i < pin.subPins.size() && subsJson.is_array() && i < subsJson.size(); ++i) {
							if (IsUnsignedId(subsJson[i], "id")) {
								pin.subPins[i].id = subsJson[i]["id"].get<uint32_t>();
							}
							if (subsJson[i].contains("value")) {
								pin.subPins[i].value = PinValueFromJson(PinType::kFloat, subsJson[i]["value"]);
							}
						}
					}
					break;
				}
			}
		};
		restorePins(node->inputs, nodeJson.value("inputs", json::array()));
		restorePins(node->outputs, nodeJson.value("outputs", json::array()));
		nodes_.push_back(std::move(node));
	}

	for (const json& linkJson : linksJson) {
		if (!IsUnsignedId(linkJson, "start") || !IsUnsignedId(linkJson, "end")) {
			continue;
		}
		const Pin* start = FindPin(linkJson["start"].get<uint32_t>());
		const Pin* end = FindPin(linkJson["end"].get<uint32_t>());
		if (!start || !end || !CanLink(*start, *end) || start->kind != PinKind::kOutput) {
			continue;
		}
		Link link;
		link.id = IsUnsignedId(linkJson, "id") ? linkJson["id"].get<uint32_t>() : GenerateId();
		link.startPinId = start->id;
		link.endPinId = end->id;
		// 入力ピンに繋がるリンクは1本だけ.
		std::erase_if(links_, [&link](const Link& other) { return other.endPinId == link.endPinId; });
		links_.push_back(link);
	}

	isChanged_ = false;
	return true;
}

/*=============================================================
描画
=============================================================*/
void GuiNode::Draw() {
#ifdef USE_IMGUI
	ImGui::SetNextWindowSize(ImVec2(900.0f, 520.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(name_.c_str())) {
		DrawContent();
	}
	ImGui::End();
#endif // USE_IMGUI
}

void GuiNode::DrawContent([[maybe_unused]] const Vector2& size) {
#ifdef USE_IMGUI
	if (!context_) {
		return;
	}
	ed::SetCurrentEditor(context_);
	// ノード内の文字入力中にDeleteキーでノードが消えないようにする.
	ed::EnableShortcuts(!ImGui::GetIO().WantTextInput);

	ed::Begin(name_.c_str(), ImVec2(size.x, size.y));

	// ノード.
	for (auto& node : nodes_) {
		if (node->isPositionDirty_) {
			ed::SetNodePosition(node->id, ImVec2(node->position.x, node->position.y));
			node->isPositionDirty_ = false;
		}
		DrawNode(*node);
	}

	// リンク.
	for (const Link& link : links_) {
		const Pin* start = FindPin(link.startPinId);
		const ImColor color = start ? GetPinColor(start->type) : ImColor(255, 255, 255);
		ed::Link(link.id, link.startPinId, link.endPinId, color, 2.0f);
	}

	UpdateCreate();
	UpdateDelete();
	DrawPopups();

	// エディター上で動かされた位置を受け取る.
	for (auto& node : nodes_) {
		if (node->isPositionDirty_) {
			// このフレームで作られたノードはまだエディターに無いので次のフレームで反映する.
			continue;
		}
		const ImVec2 position = ed::GetNodePosition(node->id);
		if (position.x == FLT_MAX || position.y == FLT_MAX) {
			continue;
		}
		if (position.x != node->position.x || position.y != node->position.y) {
			node->position = Vector2{ position.x, position.y };
			isChanged_ = true;
		}
	}

	if (isNavigateRequested_) {
		ed::NavigateToContent(0.0f);
		isNavigateRequested_ = false;
	}

	ed::End();
	ed::SetCurrentEditor(nullptr);
#endif // USE_IMGUI
}

#ifdef USE_IMGUI
void GuiNode::DrawNode(Node& node) {
	const auto typeIt = nodeTypes_.find(node.typeName);
	const NodeTypeDesc* desc = (typeIt != nodeTypes_.end()) ? &typeIt->second : nullptr;

	ed::BeginNode(node.id);
	ImGui::PushID(static_cast<int>(node.id));

	// タイトル.
	ImGui::TextUnformatted(node.title.c_str());
	const ImVec2 titleMax = ImGui::GetItemRectMax();
	ImGui::Dummy(ImVec2(0.0f, 2.0f));

	// 入力ピン(左側. 分割されていれば要素ピンを並べる).
	ImGui::BeginGroup();
	for (Pin& pin : node.inputs) {
		if (pin.isSplit) {
			for (Pin& sub : pin.subPins) {
				DrawInputPin(sub, sub.name);
			}
		} else {
			DrawInputPin(pin, pin.name);
		}
	}
	ImGui::EndGroup();

	// 出力ピン(右側. 名前を右ぞろえにする).
	if (!node.outputs.empty()) {
		if (!node.inputs.empty()) {
			ImGui::SameLine(0.0f, 24.0f);
		}
		ImGui::BeginGroup();
		float maxNameWidth = 0.0f;
		for (const Pin& pin : node.outputs) {
			if (pin.isSplit) {
				for (const Pin& sub : pin.subPins) {
					maxNameWidth = (std::max)(maxNameWidth, ImGui::CalcTextSize(sub.name.c_str()).x);
				}
			} else {
				maxNameWidth = (std::max)(maxNameWidth, ImGui::CalcTextSize(pin.name.c_str()).x);
			}
		}
		for (Pin& pin : node.outputs) {
			if (pin.isSplit) {
				for (Pin& sub : pin.subPins) {
					DrawOutputPin(sub, sub.name, maxNameWidth);
				}
			} else {
				DrawOutputPin(pin, pin.name, maxNameWidth);
			}
		}
		ImGui::EndGroup();
	}

	// 種類ごとの追加の描画.
	if (desc && desc->drawContent) {
		desc->drawContent(node);
	}

	ImGui::PopID();
	ed::EndNode();

	// タイトルの背景をノードの色で塗る.
	if (ImGui::IsItemVisible() && desc) {
		const ImVec2 nodePos = ed::GetNodePosition(node.id);
		const ImVec2 nodeSize = ed::GetNodeSize(node.id);
		const ed::Style& style = ed::GetStyle();
		const float inset = style.NodeBorderWidth * 0.5f;
		const ImVec2 headerMin(nodePos.x + inset, nodePos.y + inset);
		const ImVec2 headerMax(nodePos.x + nodeSize.x - inset, titleMax.y + 4.0f);
		ImDrawList* drawList = ed::GetNodeBackgroundDrawList(node.id);
		ImVec4 color = ToImVec4(desc->color);
		color.w *= 0.85f;
		drawList->AddRectFilled(headerMin, headerMax, ImColor(color), style.NodeRounding, ImDrawFlags_RoundCornersTop);
		drawList->AddLine(ImVec2(headerMin.x, headerMax.y), ImVec2(headerMax.x, headerMax.y), ImColor(255, 255, 255, 64), 1.0f);
	}
}

void GuiNode::DrawInputPin(Pin& pin, const std::string& label) {
	const bool isLinked = IsPinLinked(pin.id);
	ed::BeginPin(pin.id, ed::PinKind::Input);
	ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));
	ed::PinPivotSize(ImVec2(0.0f, 0.0f));
	DrawPinIcon(pin, isLinked);
	ed::EndPin();
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(label.c_str());
	// 繋がっていない入力ピンは値をその場で編集できる.
	if (!isLinked && pin.showValue && pin.type != PinType::kFlow) {
		ImGui::SameLine();
		DrawInputValue(pin);
	}
}

void GuiNode::DrawOutputPin(Pin& pin, const std::string& label, float labelWidth) {
	const float nameWidth = ImGui::CalcTextSize(label.c_str()).x;
	ImGui::Dummy(ImVec2(labelWidth - nameWidth, 0.0f));
	ImGui::SameLine(0.0f, 0.0f);
	ImGui::AlignTextToFramePadding();
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine();
	ed::BeginPin(pin.id, ed::PinKind::Output);
	ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
	ed::PinPivotSize(ImVec2(0.0f, 0.0f));
	DrawPinIcon(pin, IsPinLinked(pin.id));
	ed::EndPin();
}

void GuiNode::DrawInputValue(Pin& pin) {
	ImGui::PushID(static_cast<int>(pin.id));
	bool isChanged = false;
	switch (pin.type) {
	case PinType::kBool:
		if (bool* v = std::get_if<bool>(&pin.value)) {
			isChanged = ImGui::Checkbox("##value", v);
		}
		break;
	case PinType::kInt:
		if (int* v = std::get_if<int>(&pin.value)) {
			ImGui::SetNextItemWidth(70.0f);
			isChanged = ImGui::DragInt("##value", v);
		}
		break;
	case PinType::kFloat:
		if (float* v = std::get_if<float>(&pin.value)) {
			ImGui::SetNextItemWidth(70.0f);
			isChanged = ImGui::DragFloat("##value", v, 0.01f);
		}
		break;
	case PinType::kVector2:
		if (Vector2* v = std::get_if<Vector2>(&pin.value)) {
			ImGui::SetNextItemWidth(130.0f);
			isChanged = ImGui::DragFloat2("##value", &v->x, 0.01f);
		}
		break;
	case PinType::kVector3:
		if (Vector3* v = std::get_if<Vector3>(&pin.value)) {
			ImGui::SetNextItemWidth(190.0f);
			isChanged = ImGui::DragFloat3("##value", &v->x, 0.01f);
		}
		break;
	case PinType::kColor:
		if (Vector4* v = std::get_if<Vector4>(&pin.value)) {
			// ノードの中ではポップアップを開けないので、クリックされたらDrawPopupsで色を選ぶ.
			const float height = ImGui::GetFrameHeight();
			if (ImGui::ColorButton("##value", ToImVec4(*v), ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(height * 2.0f, height))) {
				colorPickerPinId_ = pin.id;
				isColorPickerRequested_ = true;
			}
		}
		break;
	case PinType::kString:
		if (std::string* v = std::get_if<std::string>(&pin.value)) {
			char buffer[256]{};
			strncpy_s(buffer, v->c_str(), _TRUNCATE);
			ImGui::SetNextItemWidth(120.0f);
			if (ImGui::InputText("##value", buffer, sizeof(buffer))) {
				*v = buffer;
				isChanged = true;
			}
		}
		break;
	default:
		break;
	}
	if (isChanged) {
		isChanged_ = true;
	}
	ImGui::PopID();
}

void GuiNode::DrawPinIcon(const Pin& pin, bool isLinked) const {
	const float size = ImGui::GetFrameHeight();
	ImGui::Dummy(ImVec2(size, size));
	const ImVec2 min = ImGui::GetItemRectMin();
	const ImVec2 center(min.x + size * 0.5f, min.y + size * 0.5f);
	const float radius = size * 0.28f;
	const ImColor color = GetPinColor(pin.type);
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	if (pin.type == PinType::kFlow) {
		// 処理の流れは三角形.
		const ImVec2 p1(center.x - radius, center.y - radius);
		const ImVec2 p2(center.x + radius, center.y);
		const ImVec2 p3(center.x - radius, center.y + radius);
		if (isLinked) {
			drawList->AddTriangleFilled(p1, p2, p3, color);
		} else {
			drawList->AddTriangle(p1, p2, p3, color, 2.0f);
		}
	} else if (CanSplit(pin.type)) {
		// 構造体(分割できる)ピンは四角形.
		const ImVec2 rectMin(center.x - radius, center.y - radius);
		const ImVec2 rectMax(center.x + radius, center.y + radius);
		if (isLinked) {
			drawList->AddRectFilled(rectMin, rectMax, color, 2.0f);
		} else {
			drawList->AddRect(rectMin, rectMax, color, 2.0f, 0, 2.0f);
		}
	} else {
		if (isLinked) {
			drawList->AddCircleFilled(center, radius, color);
		} else {
			drawList->AddCircle(center, radius, color, 0, 2.0f);
		}
	}
}

void GuiNode::UpdateCreate() {
	if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f)) {
		// ピン同士を繋ぐ.
		ed::PinId startPinId = 0;
		ed::PinId endPinId = 0;
		if (ed::QueryNewLink(&startPinId, &endPinId) && startPinId && endPinId) {
			const Pin* startPin = FindPin(static_cast<uint32_t>(startPinId.Get()));
			const Pin* endPin = FindPin(static_cast<uint32_t>(endPinId.Get()));
			if (startPin && endPin && CanLink(*startPin, *endPin)) {
				if (ed::AcceptNewItem(GetPinColor(startPin->type), 3.0f)) {
					AddLink(startPin->id, endPin->id);
				}
			} else {
				ed::RejectNewItem(ImColor(255, 64, 64), 2.0f);
			}
		}

		// ピンから何もない所へドラッグした → ノードを作って繋ぐ.
		ed::PinId pinId = 0;
		if (ed::QueryNewNode(&pinId)) {
			if (ed::AcceptNewItem()) {
				// ここではマウス位置がキャンバス座標にならないので、メニューはDrawPopupsで開く.
				newNodeLinkPinId_ = static_cast<uint32_t>(pinId.Get());
				isCreateMenuRequested_ = true;
			}
		}
	}
	ed::EndCreate();
}

void GuiNode::UpdateDelete() {
	if (ed::BeginDelete()) {
		ed::LinkId linkId = 0;
		while (ed::QueryDeletedLink(&linkId)) {
			if (ed::AcceptDeletedItem()) {
				RemoveLink(static_cast<uint32_t>(linkId.Get()));
			}
		}
		ed::NodeId nodeId = 0;
		while (ed::QueryDeletedNode(&nodeId)) {
			if (ed::AcceptDeletedItem()) {
				RemoveNode(static_cast<uint32_t>(nodeId.Get()));
			}
		}
	}
	ed::EndDelete();
}

void GuiNode::DrawPopups() {
	// エディターの中ではマウス位置がキャンバス座標になっている.
	const ImVec2 mousePos = ImGui::GetMousePos();

	ed::Suspend();
	ed::NodeId nodeId = 0;
	ed::PinId pinId = 0;
	ed::LinkId linkId = 0;
	if (ed::ShowNodeContextMenu(&nodeId)) {
		contextNodeId_ = static_cast<uint32_t>(nodeId.Get());
		ImGui::OpenPopup(kNodeMenuPopup);
	} else if (ed::ShowPinContextMenu(&pinId)) {
		contextPinId_ = static_cast<uint32_t>(pinId.Get());
		ImGui::OpenPopup(kPinMenuPopup);
	} else if (ed::ShowLinkContextMenu(&linkId)) {
		contextLinkId_ = static_cast<uint32_t>(linkId.Get());
		ImGui::OpenPopup(kLinkMenuPopup);
	} else if (ed::ShowBackgroundContextMenu()) {
		newNodeLinkPinId_ = 0;
		newNodePosition_ = Vector2{ mousePos.x, mousePos.y };
		ImGui::OpenPopup(kCreateMenuPopup);
	}
	if (isCreateMenuRequested_) {
		newNodePosition_ = Vector2{ mousePos.x, mousePos.y };
		ImGui::OpenPopup(kCreateMenuPopup);
		isCreateMenuRequested_ = false;
	}
	if (isColorPickerRequested_) {
		ImGui::OpenPopup(kColorPickerPopup);
		isColorPickerRequested_ = false;
	}

	// ノードの右クリックメニュー.
	if (ImGui::BeginPopup(kNodeMenuPopup)) {
		if (const Node* node = FindNode(contextNodeId_)) {
			ImGui::TextDisabled("%s", node->title.c_str());
			ImGui::Separator();
			if (ImGui::MenuItem("削除")) {
				ed::DeleteNode(contextNodeId_);
			}
		} else {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// ピンの右クリックメニュー(構造体ピンの分割).
	if (ImGui::BeginPopup(kPinMenuPopup)) {
		if (const Pin* pin = FindPin(contextPinId_)) {
			ImGui::TextDisabled("%s", pin->name.c_str());
			ImGui::Separator();
			if (pin->parentPinId != 0) {
				if (ImGui::MenuItem("分割をやめる")) {
					UnsplitPin(pin->id);
				}
			} else if (CanSplit(pin->type)) {
				if (ImGui::MenuItem("ピンを分割")) {
					SplitPin(pin->id);
				}
			} else {
				ImGui::TextDisabled("このピンは分割できません");
			}
			if (IsPinLinked(pin->id) && ImGui::MenuItem("リンクを外す")) {
				RemoveLinksOfPin(pin->id);
			}
		} else {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// リンクの右クリックメニュー.
	if (ImGui::BeginPopup(kLinkMenuPopup)) {
		if (FindLink(contextLinkId_)) {
			if (ImGui::MenuItem("削除")) {
				ed::DeleteLink(contextLinkId_);
			}
		} else {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	// ノードの作成メニュー.
	if (ImGui::BeginPopup(kCreateMenuPopup)) {
		ImGui::TextDisabled("ノードを作成");
		ImGui::Separator();
		if (nodeTypes_.empty()) {
			ImGui::TextDisabled("(RegisterNodeTypeで種類を登録してください)");
		}
		DrawCreateNodeMenu("");
		ImGui::EndPopup();
	} else {
		newNodeLinkPinId_ = 0;
	}

	// 色の入力ピンの色選択.
	if (ImGui::BeginPopup(kColorPickerPopup)) {
		Pin* pin = FindPin(colorPickerPinId_);
		Vector4* color = pin ? std::get_if<Vector4>(&pin->value) : nullptr;
		if (color) {
			if (ImGui::ColorPicker4("##picker", &color->x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)) {
				isChanged_ = true;
			}
		} else {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ed::Resume();
}

void GuiNode::DrawCreateNodeMenu(const std::string& categoryPrefix) {
	const Pin* linkPin = FindPin(newNodeLinkPinId_);

	// ピンからドラッグして開いた時は、そのピンと繋げる種類だけ出す.
	auto isCreatable = [linkPin](const NodeTypeDesc& desc) {
		if (desc.isHiddenInMenu) {
			return false;
		}
		if (!linkPin) {
			return true;
		}
		const std::vector<PinDesc>& pins = (linkPin->kind == PinKind::kOutput) ? desc.inputs : desc.outputs;
		return std::any_of(pins.begin(), pins.end(), [linkPin](const PinDesc& pin) { return pin.type == linkPin->type; });
	};

	// この階層の下にある分類(サブメニュー)を集める.
	std::vector<std::string> subMenus;
	for (const auto& [typeName, desc] : nodeTypes_) {
		const std::string& category = desc.category;
		if (category == categoryPrefix || !isCreatable(desc)) {
			continue;
		}
		std::string rest;
		if (categoryPrefix.empty()) {
			rest = category;
		} else if (category.size() > categoryPrefix.size() && category.compare(0, categoryPrefix.size(), categoryPrefix) == 0 && category[categoryPrefix.size()] == '/') {
			rest = category.substr(categoryPrefix.size() + 1);
		} else {
			continue;
		}
		const std::string subMenu = rest.substr(0, rest.find('/'));
		if (std::find(subMenus.begin(), subMenus.end(), subMenu) == subMenus.end()) {
			subMenus.push_back(subMenu);
		}
	}
	for (const std::string& subMenu : subMenus) {
		if (ImGui::BeginMenu(subMenu.c_str())) {
			DrawCreateNodeMenu(categoryPrefix.empty() ? subMenu : categoryPrefix + "/" + subMenu);
			ImGui::EndMenu();
		}
	}

	// この階層の種類.
	for (const auto& [typeName, desc] : nodeTypes_) {
		if (desc.category != categoryPrefix || !isCreatable(desc)) {
			continue;
		}
		if (ImGui::MenuItem(typeName.c_str())) {
			const uint32_t linkPinId = newNodeLinkPinId_;
			Node* node = AddNode(typeName, newNodePosition_);
			// ピンからドラッグして作った時は、最初に繋げられるピンと繋ぐ.
			if (node && linkPin && linkPinId != 0) {
				std::vector<Pin>& pins = (linkPin->kind == PinKind::kOutput) ? node->inputs : node->outputs;
				for (const Pin& pin : pins) {
					if (AddLink(linkPinId, pin.id) != 0) {
						break;
					}
				}
			}
			newNodeLinkPinId_ = 0;
		}
	}
}
#endif // USE_IMGUI
