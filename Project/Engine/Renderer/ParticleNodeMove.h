#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>
#include "../Math/Vector3.h"

class GuiNode;
struct ParticleData;

/// <summary>
/// ノード(GuiNode)で作ったパーティクルの動き.
/// ・"生成時"ノード   : パーティクルが作られた時の 位置 / 速度 / 色 / 寿命 / 大きさ を決める.
/// ・"毎フレーム"ノード : 毎フレームの 位置 / 速度 / 色 / 大きさ を決める(繋がっていないピン・要素はそのまま).
///   位置が繋がっていない要素は 速度 × ΔT で動く.
/// グラフは Resources/Node/Particle/名前.json に保存し、Get(名前)で読み込んで使う.
/// </summary>
class ParticleNodeMove {
public:
	// 保存先のフォルダ.
	static inline const std::string kDirectoryPath = "Resources/Node/Particle/";

	// ノードの値(float / Vector3 / 色 をまとめて4つのfloatで扱う).
	struct Value {
		float v[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	};

	/// <summary>
	/// パーティクル用のノードの種類をGuiNodeに登録する(エディターと読み込みで共通).
	/// </summary>
	static void RegisterNodeTypes(GuiNode& graph);

	// 新しいグラフに最初から置いておくノード(生成時 / 毎フレーム / パーティクル)を作る.
	static void CreateDefaultNodes(GuiNode& graph);

	/// <summary>
	/// 保存されている動きを取得する(初めての時はファイルから読み込む).
	/// </summary>
	/// <param name="name">.jsonを抜いたファイル名</param>
	/// <returns>読み込めなければnullptr</returns>
	static std::shared_ptr<ParticleNodeMove> Get(const std::string& name);

	/// <summary>
	/// 読み込み済みの動きをファイルから読み直す(使っている全てのパーティクルに反映される).
	/// </summary>
	static void Reload(const std::string& name);

	// 保存されている動きの名前(.json抜き)の一覧.
	static std::vector<std::string> GetSavedNames();

	// 名前 → 保存先のパス.
	static std::string GetFilePath(const std::string& name) { return kDirectoryPath + name + ".json"; };

	/// <summary>
	/// グラフから計算の手順を作る.
	/// </summary>
	void Compile(const GuiNode& graph);

	// ファイルを読み込んでCompileする.
	bool LoadFromFile(const std::string& filePath);

	/// <summary>
	/// 新しいパーティクルの初期値を決める.
	/// </summary>
	/// <param name="particle">transform.translate / transform.scale(基準の大きさ) を設定済みのパーティクル</param>
	void Spawn(ParticleData& particle) const;

	/// <summary>
	/// パーティクルを1フレーム分動かす.
	/// </summary>
	void Update(ParticleData& particle, float deltaTime) const;

private:
	// ノードの計算の種類.
	enum class Op {
		kSpawn,
		kUpdate,
		kParticle,
		kConstant,
		kRandomFloat,
		kRandomVector3,
		kRandomDirection,
		kRandomColor,
		kAdd,
		kSubtract,
		kMultiply,
		kDivide,
		kScale,
		kSin,
		kCos,
		kOneMinus,
		kClamp,
		kNormalize,
		kLength,
		kLerp,
		kEasing,
		kCount,
	};

	// 入力の1要素がどこから来るか.
	struct Source {
		int order = -1;       // 計算順の何番目のノードの出力か(-1なら定数).
		int output = 0;       // 何番目の出力ピンか.
		int component = 0;    // 出力の何番目の要素か.
		float constant = 0.0f;
	};

	struct Input {
		Source components[4];
	};

	struct CompiledNode {
		Op op = Op::kConstant;
		// 要素数(float:1 / Vector3:3 / 色:4)ごとの計算に使う.
		int componentCount = 1;
		std::vector<Input> inputs;
		int outputCount = 0;
		// outputs_ の中のこのノードの出力の先頭.
		int outputOffset = 0;
	};

	// 1つの出力ノード(生成時 / 毎フレーム)を計算するための手順.
	struct Program {
		// 計算する順番に並べたノード(最後が出力ノード).
		std::vector<CompiledNode> nodes;
		int outputCount = 0;
		bool isValid = false;
	};

	// 計算中の状態.
	struct Context {
		const ParticleData* particle = nullptr;
		float deltaTime = 0.0f;
		bool isSpawn = false;
	};

	Program BuildProgram(const GuiNode& graph, const std::string& outputTypeName) const;

	// プログラムを実行し、出力ノードの入力の値を返す.
	void Run(const Program& program, const Context& context, std::vector<Value>& outInputs, std::vector<uint8_t>& outLinked) const;

	static Op GetOp(const std::string& typeName, int& outComponentCount);

private:
	Program spawnProgram_;
	Program updateProgram_;

	static std::map<std::string, std::shared_ptr<ParticleNodeMove>>& GetCache();
};
