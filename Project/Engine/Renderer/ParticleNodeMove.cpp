#include "ParticleNodeMove.h"
#include <algorithm>
#include <filesystem>
#include <unordered_map>
#include <cmath>
#include "Particles.h"
#include "../../Managers/GuiNode.h"
#include "../Math/Random.h"
#include "../Math/Easing.h"

namespace {
using PinType = GuiNode::PinType;
using PinDesc = GuiNode::PinDesc;

/*=============================================================
ノードの種類の名前(保存ファイルにも書かれるので変えないこと)
=============================================================*/
const char* const kSpawnNode = "生成時";
const char* const kUpdateNode = "毎フレーム";
const char* const kParticleNode = "パーティクル";

// "パーティクル"ノードの出力ピンの番号.
enum ParticleOutput {
	kStartPosition, // 生成位置.
	kPosition,      // 位置.
	kVelocity,      // 速度.
	kStartVelocity, // 初速.
	kColor,         // 色.
	kStartColor,    // 初期色.
	kScaleRate,     // 大きさ(倍率).
	kTime,          // 経過時間.
	kLifeTime,      // 寿命.
	kRate,          // 割合(0〜1).
	kRandom,        // 乱数(パーティクルごとに固定).
	kDeltaTime,     // ΔT.
	kTarget,        // 目標地点(生成位置 + MakeNewParticleで渡した目標地点).
	kParticleOutputCount,
};

// 出力ノード(生成時 / 毎フレーム)の入力ピンの番号.
enum OutputNodeInput {
	kInPosition,
	kInVelocity,
	kInColor,
	kInLifeTime, // 生成時のみ.
	kInScale,    // 毎フレームでは3番目.
};

const Vector4 kOutputColor{ 0.85f, 0.45f, 0.20f, 1.0f };
const Vector4 kInputColor{ 0.25f, 0.55f, 0.85f, 1.0f };
const Vector4 kConstantColor{ 0.35f, 0.60f, 0.35f, 1.0f };
const Vector4 kRandomColor{ 0.65f, 0.40f, 0.75f, 1.0f };
const Vector4 kMathColor{ 0.40f, 0.45f, 0.55f, 1.0f };
const Vector4 kLerpColor{ 0.70f, 0.60f, 0.25f, 1.0f };

const Vector3 kZero3{ 0.0f, 0.0f, 0.0f };
const Vector3 kOne3{ 1.0f, 1.0f, 1.0f };
const Vector4 kWhite{ 1.0f, 1.0f, 1.0f, 1.0f };

PinDesc F(const char* name, float value = 0.0f) { return { name, PinType::kFloat, value }; }
PinDesc V3(const char* name, const Vector3& value = kZero3) { return { name, PinType::kVector3, value }; }
PinDesc C(const char* name, const Vector4& value = kWhite) { return { name, PinType::kColor, value }; }
// 値をノード上で編集しないピン(繋がっていなければ「変更しない」).
PinDesc NoValue(PinType type, const char* name) { return { name, type, {}, false }; }

float Random01() {
	return Random::GetInstance()->RandomFloat(0.0f, 1.0f);
}

// パーティクルの種 / ノード / 要素 から決まる0〜1の値(同じ組み合わせなら毎回同じ値).
float FixedRandom01(uint32_t seed, uint32_t salt, uint32_t component) {
	uint32_t h = seed ^ (salt * 0x9E3779B9u) ^ (component * 0x85EBCA6Bu);
	h ^= h >> 16;
	h *= 0x7FEB352Du;
	h ^= h >> 15;
	h *= 0x846CA68Bu;
	h ^= h >> 16;
	return static_cast<float>(h >> 8) * (1.0f / 16777216.0f);
}

int GetPinComponentCount(PinType type) {
	switch (type) {
	case PinType::kVector2: return 2;
	case PinType::kVector3: return 3;
	case PinType::kColor:   return 4;
	default:                return 1;
	}
}

// ピンの値 → 定数.
ParticleNodeMove::Value ToValue(const GuiNode::PinValue& pinValue) {
	ParticleNodeMove::Value value;
	if (const float* f = std::get_if<float>(&pinValue)) {
		value.v[0] = *f;
	} else if (const int* i = std::get_if<int>(&pinValue)) {
		value.v[0] = static_cast<float>(*i);
	} else if (const bool* b = std::get_if<bool>(&pinValue)) {
		value.v[0] = *b ? 1.0f : 0.0f;
	} else {
		for (int c = 0; c < 4; ++c) {
			value.v[c] = GuiNode::GetComponent(pinValue, c);
		}
	}
	return value;
}

std::filesystem::path ToPath(const std::string& utf8) {
	return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(utf8.data()), utf8.size()));
}
} // namespace

/*=============================================================
ノードの種類
=============================================================*/
struct ParticleNodeTypeDef {
	GuiNode::NodeTypeDesc desc;
	int op;
	int componentCount;
};

static const std::vector<ParticleNodeTypeDef>& GetNodeTypeDefs() {
	static const std::vector<ParticleNodeTypeDef> defs = [] {
		std::vector<ParticleNodeTypeDef> result;
		auto add = [&result](const char* name, const char* category, const Vector4& color, int op, int componentCount,
			std::vector<PinDesc> inputs, std::vector<PinDesc> outputs) {
			GuiNode::NodeTypeDesc desc;
			desc.name = name;
			desc.category = category;
			desc.color = color;
			desc.inputs = std::move(inputs);
			desc.outputs = std::move(outputs);
			result.push_back({ std::move(desc), op, componentCount });
		};
		// opの番号は ParticleNodeMove::Op と同じ順番.
		enum { kSpawn, kUpdate, kParticle, kConstant, kRandomFloat, kRandomVector3, kRandomDirection, kRandomColorOp,
			kAdd, kSubtract, kMultiply, kDivide, kScale, kSin, kCos, kOneMinus, kClamp, kNormalize, kLength, kLerp, kEasing,
			kFixedRandomFloat, kFixedRandomVector3, kFixedRandomDirection, kLifeRate };

		// 出力.
		add(kSpawnNode, "出力", kOutputColor, kSpawn, 1,
			{ NoValue(PinType::kVector3, "位置"), V3("速度"), C("色"), F("寿命", 1.0f), V3("大きさ", kOne3) }, {});
		add(kUpdateNode, "出力", kOutputColor, kUpdate, 1,
			{ NoValue(PinType::kVector3, "位置"), NoValue(PinType::kVector3, "速度"), NoValue(PinType::kColor, "色"), NoValue(PinType::kVector3, "大きさ") }, {});

		// 入力.
		add(kParticleNode, "入力", kInputColor, kParticle, 1, {},
			{ V3("生成位置"), V3("位置"), V3("速度"), V3("初速"), C("色"), C("初期色"), V3("大きさ"),
			  F("経過時間"), F("寿命"), F("割合"), F("乱数"), F("ΔT"), V3("目標地点") });
		// 経過時間 ÷ 寿命 (0〜1). 繋がっていない入力はそのパーティクルの値を使う.
		add("寿命の割合", "入力", kInputColor, kLifeRate, 1,
			{ NoValue(PinType::kFloat, "経過時間"), NoValue(PinType::kFloat, "寿命") }, { F("割合"), F("残り(1-割合)") });

		// 定数.
		add("Float", "定数", kConstantColor, kConstant, 1, { F("値") }, { F("出力") });
		add("Vector3", "定数", kConstantColor, kConstant, 3, { V3("値") }, { V3("出力") });
		add("色", "定数", kConstantColor, kConstant, 4, { C("値") }, { C("出力") });

		// 乱数.
		add("乱数 Float", "乱数", kRandomColor, kRandomFloat, 1, { F("最小", 0.0f), F("最大", 1.0f) }, { F("値") });
		add("乱数 Vector3", "乱数", kRandomColor, kRandomVector3, 3, { V3("最小", { -1.0f, -1.0f, -1.0f }), V3("最大", kOne3) }, { V3("値") });
		add("乱数 方向", "乱数", kRandomColor, kRandomDirection, 3, { F("長さ", 1.0f) }, { V3("方向") });
		add("乱数 色", "乱数", kRandomColor, kRandomColorOp, 4, { C("A"), C("B", { 0.0f, 0.0f, 0.0f, 1.0f }) }, { C("色") });
		// 生成時に一度だけ決まる乱数(同じパーティクルなら毎フレーム同じ値. ノードごとに別の値).
		add("生成時乱数 Float", "乱数/生成時", kRandomColor, kFixedRandomFloat, 1, { F("最小", 0.0f), F("最大", 1.0f) }, { F("値") });
		add("生成時乱数 Vector3", "乱数/生成時", kRandomColor, kFixedRandomVector3, 3, { V3("最小", { -1.0f, -1.0f, -1.0f }), V3("最大", kOne3) }, { V3("値") });
		add("生成時乱数 方向", "乱数/生成時", kRandomColor, kFixedRandomDirection, 3, { F("長さ", 1.0f) }, { V3("方向") });

		// 演算(Float).
		add("加算 (Float)", "演算/Float", kMathColor, kAdd, 1, { F("A"), F("B") }, { F("結果") });
		add("減算 (Float)", "演算/Float", kMathColor, kSubtract, 1, { F("A"), F("B") }, { F("結果") });
		add("乗算 (Float)", "演算/Float", kMathColor, kMultiply, 1, { F("A"), F("B", 1.0f) }, { F("結果") });
		add("除算 (Float)", "演算/Float", kMathColor, kDivide, 1, { F("A"), F("B", 1.0f) }, { F("結果") });
		add("Sin", "演算/Float", kMathColor, kSin, 1, { F("値") }, { F("結果") });
		add("Cos", "演算/Float", kMathColor, kCos, 1, { F("値") }, { F("結果") });
		add("1 - x", "演算/Float", kMathColor, kOneMinus, 1, { F("値") }, { F("結果") });
		add("Clamp", "演算/Float", kMathColor, kClamp, 1, { F("値"), F("最小", 0.0f), F("最大", 1.0f) }, { F("結果") });

		// 演算(Vector3).
		add("加算 (Vector3)", "演算/Vector3", kMathColor, kAdd, 3, { V3("A"), V3("B") }, { V3("結果") });
		add("減算 (Vector3)", "演算/Vector3", kMathColor, kSubtract, 3, { V3("A"), V3("B") }, { V3("結果") });
		add("乗算 (Vector3)", "演算/Vector3", kMathColor, kMultiply, 3, { V3("A"), V3("B", kOne3) }, { V3("結果") });
		add("スケール (Vector3 × Float)", "演算/Vector3", kMathColor, kScale, 3, { V3("ベクトル"), F("倍率", 1.0f) }, { V3("結果") });
		add("正規化", "演算/Vector3", kMathColor, kNormalize, 3, { V3("ベクトル") }, { V3("結果") });
		add("長さ", "演算/Vector3", kMathColor, kLength, 3, { V3("ベクトル") }, { F("長さ") });

		// 演算(色).
		add("乗算 (色)", "演算/色", kMathColor, kMultiply, 4, { C("A"), C("B") }, { C("結果") });

		// 補間.
		add("Lerp (Float)", "補間", kLerpColor, kLerp, 1, { F("A"), F("B", 1.0f), F("割合") }, { F("結果") });
		add("Lerp (Vector3)", "補間", kLerpColor, kLerp, 3, { V3("A"), V3("B", kOne3), F("割合") }, { V3("結果") });
		add("Lerp (色)", "補間", kLerpColor, kLerp, 4, { C("A"), C("B", { 1.0f, 1.0f, 1.0f, 0.0f }), F("割合") }, { C("結果") });
		add("イージング", "補間", kLerpColor, kEasing, 1, { F("割合"), { "種類(0〜5)", PinType::kInt, 1 } }, { F("割合") });
		return result;
	}();
	return defs;
}

void ParticleNodeMove::RegisterNodeTypes(GuiNode& graph) {
	for (const ParticleNodeTypeDef& def : GetNodeTypeDefs()) {
		graph.RegisterNodeType(def.desc);
	}
}

void ParticleNodeMove::CreateDefaultNodes(GuiNode& graph) {
	GuiNode::Node* particle = graph.AddNode(kParticleNode, { -320.0f, 0.0f });
	GuiNode::Node* spawn = graph.AddNode(kSpawnNode, { 200.0f, -80.0f });
	graph.AddNode(kUpdateNode, { 200.0f, 140.0f });
	// 最初から MakeNewParticleで渡した 速度 / 寿命 をそのまま使うように繋いでおく.
	if (particle && spawn) {
		graph.AddLink(particle->outputs[kStartVelocity].id, spawn->inputs[kInVelocity].id);
		graph.AddLink(particle->outputs[kLifeTime].id, spawn->inputs[kInLifeTime].id);
	}
}

ParticleNodeMove::Op ParticleNodeMove::GetOp(const std::string& typeName, int& outComponentCount) {
	static const std::unordered_map<std::string, std::pair<int, int>> table = [] {
		std::unordered_map<std::string, std::pair<int, int>> result;
		for (const ParticleNodeTypeDef& def : GetNodeTypeDefs()) {
			result[def.desc.name] = { def.op, def.componentCount };
		}
		return result;
	}();
	auto it = table.find(typeName);
	if (it == table.end()) {
		outComponentCount = 1;
		return Op::kConstant;
	}
	outComponentCount = it->second.second;
	return static_cast<Op>(it->second.first);
}

/*=============================================================
読み込み / キャッシュ
=============================================================*/
std::map<std::string, std::shared_ptr<ParticleNodeMove>>& ParticleNodeMove::GetCache() {
	static std::map<std::string, std::shared_ptr<ParticleNodeMove>> cache;
	return cache;
}

std::shared_ptr<ParticleNodeMove> ParticleNodeMove::Get(const std::string& name) {
	auto& cache = GetCache();
	auto it = cache.find(name);
	if (it != cache.end()) {
		return it->second;
	}
	auto move = std::make_shared<ParticleNodeMove>();
	if (!move->LoadFromFile(GetFilePath(name))) {
		return nullptr;
	}
	cache[name] = move;
	return move;
}

void ParticleNodeMove::Reload(const std::string& name) {
	auto& cache = GetCache();
	auto it = cache.find(name);
	if (it != cache.end()) {
		it->second->LoadFromFile(GetFilePath(name));
	}
}

std::vector<std::string> ParticleNodeMove::GetSavedNames() {
	std::vector<std::string> names;
	std::error_code ec;
	const std::filesystem::path directory = ToPath(kDirectoryPath);
	if (!std::filesystem::is_directory(directory, ec)) {
		return names;
	}
	for (const auto& entry : std::filesystem::directory_iterator(directory, ec)) {
		if (entry.is_regular_file() && entry.path().extension() == ".json") {
			const std::u8string stem = entry.path().stem().u8string();
			names.emplace_back(reinterpret_cast<const char*>(stem.data()), stem.size());
		}
	}
	std::sort(names.begin(), names.end());
	return names;
}

bool ParticleNodeMove::LoadFromFile(const std::string& filePath) {
	GuiNode graph;
	RegisterNodeTypes(graph);
	if (!graph.Load(filePath)) {
		return false;
	}
	Compile(graph);
	return true;
}

/*=============================================================
グラフ → 計算の手順
=============================================================*/
void ParticleNodeMove::Compile(const GuiNode& graph) {
	spawnProgram_ = BuildProgram(graph, kSpawnNode);
	updateProgram_ = BuildProgram(graph, kUpdateNode);
}

ParticleNodeMove::Program ParticleNodeMove::BuildProgram(const GuiNode& graph, const std::string& outputTypeName) const {
	Program program;
	const GuiNode::Node* outputNode = nullptr;
	for (const auto& node : graph.GetNodes()) {
		if (node->typeName == outputTypeName) {
			outputNode = node.get();
			break;
		}
	}
	if (!outputNode) {
		return program;
	}

	// 出力ノードから入力側へたどり、計算する順番(入力が先)に並べる. ループしているリンクは無視する.
	std::vector<const GuiNode::Node*> order;
	std::unordered_map<uint32_t, int> state; // 1:たどっている途中 / 2:並べ終わった.
	auto visit = [&](auto&& self, const GuiNode::Node* node) -> void {
		state[node->id] = 1;
		auto visitPin = [&](const GuiNode::Pin& pin) {
			const GuiNode::Pin* output = graph.GetLinkedOutputPin(pin.id);
			if (!output) { return; }
			const GuiNode::Node* source = graph.FindNode(output->nodeId);
			if (source && state[source->id] == 0) {
				self(self, source);
			}
		};
		for (const GuiNode::Pin& pin : node->inputs) {
			if (pin.isSplit) {
				for (const GuiNode::Pin& sub : pin.subPins) { visitPin(sub); }
			} else {
				visitPin(pin);
			}
		}
		state[node->id] = 2;
		order.push_back(node);
	};
	visit(visit, outputNode);

	std::unordered_map<uint32_t, int> orderIndex;
	for (int i = 0; i < static_cast<int>(order.size()); ++i) {
		orderIndex[order[i]->id] = i;
	}

	// 入力ピン(の要素)がどの出力から来るかを決める.
	auto resolve = [&](const GuiNode::Pin& pin, int componentCount, Input& input, int baseComponent, int currentOrder) {
		const GuiNode::Pin* output = graph.GetLinkedOutputPin(pin.id);
		const GuiNode::Node* source = output ? graph.FindNode(output->nodeId) : nullptr;
		auto sourceIt = source ? orderIndex.find(source->id) : orderIndex.end();
		// 自分より後に計算するノード(ループ)からの値は使わない.
		if (!output || sourceIt == orderIndex.end() || sourceIt->second >= currentOrder) {
			const Value constant = ToValue(pin.value);
			for (int c = 0; c < componentCount; ++c) {
				input.components[baseComponent + c].constant = constant.v[c];
			}
			return;
		}
		// 出力ピンの番号(出力側が分割されていれば元のピンの番号と要素).
		uint32_t outputPinId = output->parentPinId != 0 ? output->parentPinId : output->id;
		int outputIndex = 0;
		for (int i = 0; i < static_cast<int>(source->outputs.size()); ++i) {
			if (source->outputs[i].id == outputPinId) {
				outputIndex = i;
				break;
			}
		}
		for (int c = 0; c < componentCount; ++c) {
			Source& s = input.components[baseComponent + c];
			s.order = sourceIt->second;
			s.output = outputIndex;
			s.component = output->parentPinId != 0 ? output->componentIndex : c;
		}
	};

	int outputOffset = 0;
	for (int i = 0; i < static_cast<int>(order.size()); ++i) {
		const GuiNode::Node* node = order[i];
		CompiledNode compiled;
		compiled.op = GetOp(node->typeName, compiled.componentCount);
		compiled.salt = node->id;
		for (const GuiNode::Pin& pin : node->inputs) {
			Input input;
			if (pin.isSplit) {
				for (const GuiNode::Pin& sub : pin.subPins) {
					resolve(sub, 1, input, sub.componentIndex, i);
				}
			} else {
				resolve(pin, GetPinComponentCount(pin.type), input, 0, i);
			}
			compiled.inputs.push_back(input);
		}
		compiled.outputCount = static_cast<int>(node->outputs.size());
		compiled.outputOffset = outputOffset;
		outputOffset += compiled.outputCount;
		program.nodes.push_back(std::move(compiled));
	}
	program.outputCount = outputOffset;
	program.isValid = true;
	return program;
}

/*=============================================================
計算
=============================================================*/
void ParticleNodeMove::Run(const Program& program, const Context& context, std::vector<Value>& outInputs, std::vector<uint8_t>& outLinked) const {
	thread_local std::vector<Value> outputs;
	outputs.assign(program.outputCount, Value{});

	const ParticleData& particle = *context.particle;
	Value in[8];
	for (const CompiledNode& node : program.nodes) {
		const int inputCount = (std::min)(static_cast<int>(node.inputs.size()), 8);
		for (int k = 0; k < inputCount; ++k) {
			for (int c = 0; c < 4; ++c) {
				const Source& s = node.inputs[k].components[c];
				in[k].v[c] = (s.order < 0) ? s.constant : outputs[program.nodes[s.order].outputOffset + s.output].v[s.component];
			}
		}
		Value* out = outputs.data() + node.outputOffset;
		const int n = node.componentCount;

		switch (node.op) {
		case Op::kSpawn:
		case Op::kUpdate:
			outInputs.assign(in, in + inputCount);
			outLinked.assign(static_cast<size_t>(inputCount) * 4, 0);
			for (int k = 0; k < inputCount; ++k) {
				for (int c = 0; c < 4; ++c) {
					outLinked[k * 4 + c] = node.inputs[k].components[c].order >= 0 ? 1 : 0;
				}
			}
			break;
		case Op::kParticle: {
			auto set3 = [&out](int index, const Vector3& v) { out[index].v[0] = v.x; out[index].v[1] = v.y; out[index].v[2] = v.z; };
			auto set4 = [&out](int index, const Vector4& v) { out[index].v[0] = v.x; out[index].v[1] = v.y; out[index].v[2] = v.z; out[index].v[3] = v.w; };
			set3(kStartPosition, particle.startPosition);
			set3(kPosition, particle.transform.translate);
			// 生成時は 速度 / 初速 / 寿命 にMakeNewParticleで渡された値が入っている.
			set3(kVelocity, particle.velocity);
			set3(kStartVelocity, particle.startVelocity);
			set4(kColor, context.isSpawn ? kWhite : particle.color);
			set4(kStartColor, context.isSpawn ? kWhite : particle.startColor);
			set3(kScaleRate, context.isSpawn ? kOne3 : particle.scaleRate);
			out[kTime].v[0] = context.isSpawn ? 0.0f : particle.currentTime;
			out[kLifeTime].v[0] = particle.lifeTime;
			out[kRate].v[0] = (context.isSpawn || particle.lifeTime <= 0.0f) ? 0.0f : std::clamp(particle.currentTime / particle.lifeTime, 0.0f, 1.0f);
			out[kRandom].v[0] = particle.random;
			out[kDeltaTime].v[0] = context.deltaTime;
			// 目標地点は生成位置から見たローカル座標で渡されるので、生成位置を足してワールド座標にする.
			set3(kTarget, {
				particle.startPosition.x + particle.targetTransform.translate.x,
				particle.startPosition.y + particle.targetTransform.translate.y,
				particle.startPosition.z + particle.targetTransform.translate.z });
			break;
		}
		case Op::kLifeRate: {
			// 繋がっていない入力はそのパーティクルの 経過時間 / 寿命.
			const float time = node.inputs[0].components[0].order >= 0 ? in[0].v[0] : (context.isSpawn ? 0.0f : particle.currentTime);
			const float lifeTime = node.inputs[1].components[0].order >= 0 ? in[1].v[0] : particle.lifeTime;
			const float rate = (lifeTime > 0.0f) ? std::clamp(time / lifeTime, 0.0f, 1.0f) : 0.0f;
			out[0].v[0] = rate;
			out[1].v[0] = 1.0f - rate;
			break;
		}
		case Op::kFixedRandomFloat:
			out[0].v[0] = in[0].v[0] + (in[1].v[0] - in[0].v[0]) * FixedRandom01(particle.randomSeed, node.salt, 0);
			break;
		case Op::kFixedRandomVector3:
			for (int c = 0; c < 3; ++c) {
				out[0].v[c] = in[0].v[c] + (in[1].v[c] - in[0].v[c]) * FixedRandom01(particle.randomSeed, node.salt, static_cast<uint32_t>(c));
			}
			break;
		case Op::kFixedRandomDirection: {
			// 球面上に均等になるよう z と角度から作る.
			const float z = FixedRandom01(particle.randomSeed, node.salt, 0) * 2.0f - 1.0f;
			const float angle = FixedRandom01(particle.randomSeed, node.salt, 1) * 6.28318530f;
			const float radius = std::sqrt((std::max)(0.0f, 1.0f - z * z));
			out[0].v[0] = std::cos(angle) * radius * in[0].v[0];
			out[0].v[1] = std::sin(angle) * radius * in[0].v[0];
			out[0].v[2] = z * in[0].v[0];
			break;
		}
		case Op::kConstant:
			out[0] = in[0];
			break;
		case Op::kRandomFloat:
			out[0].v[0] = in[0].v[0] + (in[1].v[0] - in[0].v[0]) * Random01();
			break;
		case Op::kRandomVector3:
			for (int c = 0; c < 3; ++c) {
				out[0].v[c] = in[0].v[c] + (in[1].v[c] - in[0].v[c]) * Random01();
			}
			break;
		case Op::kRandomDirection: {
			// 球の中の点を選んで正規化する(方向がかたよらないように).
			float x = 0.0f, y = 1.0f, z = 0.0f, lengthSq = 1.0f;
			for (int tryCount = 0; tryCount < 16; ++tryCount) {
				x = Random01() * 2.0f - 1.0f;
				y = Random01() * 2.0f - 1.0f;
				z = Random01() * 2.0f - 1.0f;
				lengthSq = x * x + y * y + z * z;
				if (lengthSq > 0.0001f && lengthSq <= 1.0f) { break; }
			}
			const float scale = in[0].v[0] / std::sqrt((std::max)(lengthSq, 0.0001f));
			out[0].v[0] = x * scale;
			out[0].v[1] = y * scale;
			out[0].v[2] = z * scale;
			break;
		}
		case Op::kRandomColor: {
			const float t = Random01();
			for (int c = 0; c < 4; ++c) {
				out[0].v[c] = in[0].v[c] + (in[1].v[c] - in[0].v[c]) * t;
			}
			break;
		}
		case Op::kAdd:
			for (int c = 0; c < n; ++c) { out[0].v[c] = in[0].v[c] + in[1].v[c]; }
			break;
		case Op::kSubtract:
			for (int c = 0; c < n; ++c) { out[0].v[c] = in[0].v[c] - in[1].v[c]; }
			break;
		case Op::kMultiply:
			for (int c = 0; c < n; ++c) { out[0].v[c] = in[0].v[c] * in[1].v[c]; }
			break;
		case Op::kDivide:
			for (int c = 0; c < n; ++c) { out[0].v[c] = (in[1].v[c] != 0.0f) ? in[0].v[c] / in[1].v[c] : 0.0f; }
			break;
		case Op::kScale:
			for (int c = 0; c < 3; ++c) { out[0].v[c] = in[0].v[c] * in[1].v[0]; }
			break;
		case Op::kSin:
			out[0].v[0] = std::sin(in[0].v[0]);
			break;
		case Op::kCos:
			out[0].v[0] = std::cos(in[0].v[0]);
			break;
		case Op::kOneMinus:
			out[0].v[0] = 1.0f - in[0].v[0];
			break;
		case Op::kClamp:
			out[0].v[0] = std::clamp(in[0].v[0], (std::min)(in[1].v[0], in[2].v[0]), (std::max)(in[1].v[0], in[2].v[0]));
			break;
		case Op::kNormalize: {
			const float length = std::sqrt(in[0].v[0] * in[0].v[0] + in[0].v[1] * in[0].v[1] + in[0].v[2] * in[0].v[2]);
			for (int c = 0; c < 3; ++c) { out[0].v[c] = (length > 0.0f) ? in[0].v[c] / length : 0.0f; }
			break;
		}
		case Op::kLength:
			out[0].v[0] = std::sqrt(in[0].v[0] * in[0].v[0] + in[0].v[1] * in[0].v[1] + in[0].v[2] * in[0].v[2]);
			break;
		case Op::kLerp:
			for (int c = 0; c < n; ++c) { out[0].v[c] = in[0].v[c] + (in[1].v[c] - in[0].v[c]) * in[2].v[0]; }
			break;
		case Op::kEasing: {
			const float t = std::clamp(in[0].v[0], 0.0f, 1.0f);
			const int type = std::clamp(static_cast<int>(in[1].v[0]), 0, static_cast<int>(EaseType::kEaseOutBack));
			out[0].v[0] = Easing(0.0f, 1.0f, t, 1.0f, static_cast<EaseType>(type));
			break;
		}
		default:
			break;
		}
	}
}

void ParticleNodeMove::Spawn(ParticleData& particle) const {
	particle.random = Random01();
	// "生成時乱数"ノードの種(パーティクルごとに一度だけ決める).
	particle.randomSeed = static_cast<uint32_t>(Random01() * 4294967040.0f);
	particle.startPosition = particle.transform.translate;
	particle.baseScale = particle.transform.scale;
	particle.scaleRate = kOne3;
	// MakeNewParticleで渡された 速度 / 寿命 を 初速 / 寿命 として残しておく(グラフの"パーティクル"ノードで使える).
	particle.startVelocity = particle.velocity;
	particle.startLifeTime = particle.lifeTime;
	particle.color = kWhite;
	particle.currentTime = 0.0f;

	if (spawnProgram_.isValid) {
		thread_local std::vector<Value> inputs;
		thread_local std::vector<uint8_t> linked;
		Context context{ &particle, 0.0f, true };
		Run(spawnProgram_, context, inputs, linked);
		if (inputs.size() >= 5) {
			float* translate = &particle.transform.translate.x;
			for (int c = 0; c < 3; ++c) {
				// 位置が繋がっていない要素は生成位置のまま.
				if (linked[kInPosition * 4 + c]) { translate[c] = inputs[kInPosition].v[c]; }
			}
			particle.velocity = { inputs[kInVelocity].v[0], inputs[kInVelocity].v[1], inputs[kInVelocity].v[2] };
			particle.color = { inputs[kInColor].v[0], inputs[kInColor].v[1], inputs[kInColor].v[2], inputs[kInColor].v[3] };
			particle.lifeTime = (std::max)(inputs[kInLifeTime].v[0], 0.001f);
			particle.scaleRate = { inputs[kInScale].v[0], inputs[kInScale].v[1], inputs[kInScale].v[2] };
		}
	}

	particle.startColor = particle.color;
	particle.transform.scale = { particle.baseScale.x * particle.scaleRate.x, particle.baseScale.y * particle.scaleRate.y, particle.baseScale.z * particle.scaleRate.z };
}

void ParticleNodeMove::Update(ParticleData& particle, float deltaTime) const {
	particle.currentTime += deltaTime;

	float* translate = &particle.transform.translate.x;
	float* velocity = &particle.velocity.x;

	if (!updateProgram_.isValid) {
		for (int c = 0; c < 3; ++c) { translate[c] += velocity[c] * deltaTime; }
		return;
	}

	thread_local std::vector<Value> inputs;
	thread_local std::vector<uint8_t> linked;
	Context context{ &particle, deltaTime, false };
	Run(updateProgram_, context, inputs, linked);
	if (inputs.size() < 4) {
		return;
	}
	// 毎フレームの入力ピン: 位置 / 速度 / 色 / 大きさ(繋がっている要素だけ変更する).
	const int kUpdateScale = 3;
	for (int c = 0; c < 3; ++c) {
		if (linked[kInVelocity * 4 + c]) { velocity[c] = inputs[kInVelocity].v[c]; }
	}
	for (int c = 0; c < 3; ++c) {
		if (linked[kInPosition * 4 + c]) {
			translate[c] = inputs[kInPosition].v[c];
		} else {
			translate[c] += velocity[c] * deltaTime;
		}
	}
	float* color = &particle.color.x;
	for (int c = 0; c < 4; ++c) {
		if (linked[kInColor * 4 + c]) { color[c] = inputs[kInColor].v[c]; }
	}
	float* scaleRate = &particle.scaleRate.x;
	float* scale = &particle.transform.scale.x;
	const float* baseScale = &particle.baseScale.x;
	for (int c = 0; c < 3; ++c) {
		if (linked[kUpdateScale * 4 + c]) {
			scaleRate[c] = inputs[kUpdateScale].v[c];
			scale[c] = baseScale[c] * scaleRate[c];
		}
	}
}
