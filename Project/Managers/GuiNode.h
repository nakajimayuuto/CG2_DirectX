#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <functional>
#include "../Engine/Math/Vector2.h"
#include "../Engine/Math/Vector3.h"
#include "../Engine/Math/Vector4.h"

// imgui-node-editorの型(ヘッダーではimgui_node_editor.hを読み込まないよう前方宣言だけしておく).
namespace ax::NodeEditor {
struct EditorContext;
}

/// <summary>
/// imgui-node-editor(externals/imgui_node)を使った汎用ノードエディター.
/// ・RegisterNodeType()でノードの種類(ピンの構成・色・分類)を登録する.
/// ・エディター上で右クリック → 登録した種類からノードを作成できる.
/// ・ピン同士をドラッグで接続(型が同じ 入力⇔出力 のみ)、Deleteキー / 右クリックで削除できる.
/// ・繋がっていない入力ピンはノード上で値を直接編集できる.
/// ・構造体のピン(Vector2 / Vector3 / Color)は右クリック →「ピンを分割」で要素ごとのfloatピンに分けられる.
/// ・Save() / Load()でグラフをJSONに保存・読み込みできる(Release版でも使える).
/// パーティクルエディターなどは、このクラスを持ってノードの種類を登録し、
/// GetNodes() / GetLinkedOutputPin()などでグラフをたどって使う.
/// </summary>
class GuiNode {
public:
	/*=============================================================
	ピン / ノード / リンクのデータ
	=============================================================*/
	// ピンの向き.
	enum class PinKind {
		kInput,  // 入力(ノードの左側).
		kOutput, // 出力(ノードの右側).
	};

	// ピンの型. 同じ型同士しか繋げない.
	enum class PinType {
		kFlow,    // 処理の流れ(値を持たない).
		kBool,
		kInt,
		kFloat,
		kVector2, // 構造体(分割できる).
		kVector3, // 構造体(分割できる).
		kColor,   // Vector4(RGBA). 構造体(分割できる).
		kString,
	};

	// ピンが持つ値(PinTypeに対応する型が入る. kFlowはstd::monostate).
	using PinValue = std::variant<std::monostate, bool, int, float, Vector2, Vector3, Vector4, std::string>;

	struct Pin {
		uint32_t id = 0;
		uint32_t nodeId = 0;
		std::string name;
		PinKind kind = PinKind::kInput;
		PinType type = PinType::kFlow;
		// 入力ピン: 繋がっていない時に使う値(ノード上で編集できる).
		// 出力ピン: 自由に使ってよい値(エディターでは編集しない).
		PinValue value;
		// 繋がっていない時にノード上で値を編集できるようにするか.
		bool showValue = true;

		// 分割されているか. 分割中はこのピンの代わりに subPins を表示・接続する.
		bool isSplit = false;
		// 分割した要素ごとのピン(kFloat. Vector3ならX,Y,Z).
		std::vector<Pin> subPins;
		// 要素ピンの時: 元のピンのIDと要素番号(0〜3). 要素ピンでなければ 0 / -1.
		uint32_t parentPinId = 0;
		int componentIndex = -1;
	};

	struct Node {
		uint32_t id = 0;
		// RegisterNodeType()で登録した種類の名前.
		std::string typeName;
		// ノードの上部に表示する名前(最初はtypeNameと同じ. 自由に変えてよい).
		std::string title;
		std::vector<Pin> inputs;
		std::vector<Pin> outputs;
		// エディター上の位置(キャンバス座標). 変更する時はSetNodePosition()を使う.
		Vector2 position{};
		// ノードごとに自由に使える文字列(保存される).
		std::string userData;

	private:
		friend class GuiNode;
		// positionをエディターに反映する必要があるか.
		bool isPositionDirty_ = true;
	};

	// 出力ピン(startPinId)から入力ピン(endPinId)へのリンク.
	struct Link {
		uint32_t id = 0;
		uint32_t startPinId = 0;
		uint32_t endPinId = 0;
	};

	/*=============================================================
	ノードの種類
	=============================================================*/
	struct PinDesc {
		std::string name;
		PinType type = PinType::kFlow;
		// 初期値(空ならPinTypeの既定値: 0 / false / 白 など).
		PinValue defaultValue{};
		// 繋がっていない時にノード上で値を編集できるようにするか.
		bool showValue = true;
	};

	struct NodeTypeDesc {
		// 種類の名前(エディター内で一意. 保存にも使う).
		std::string name;
		// 作成メニューでの分類("Emitter"や"Color/Gradient"のように / で階層にできる. 空ならメニューの直下).
		std::string category;
		// ノードの上部の色(RGBA, 0〜1).
		Vector4 color{ 0.30f, 0.45f, 0.70f, 1.0f };
		std::vector<PinDesc> inputs;
		std::vector<PinDesc> outputs;
		// ピンの下に追加でImGuiを描画したい時に設定する(ノード内なのでポップアップ系は使えない).
		std::function<void(Node&)> drawContent;
		// 作成メニューに出さない(コードからだけ作るノード用).
		bool isHiddenInMenu = false;
	};

public:
	GuiNode() = default;
	~GuiNode();
	GuiNode(const GuiNode&) = delete;
	GuiNode& operator=(const GuiNode&) = delete;

	/// <summary>
	/// 初期化. name はウィンドウ名とエディターのIDに使う.
	/// </summary>
	void Initialize(const std::string& name);

	/// <summary>
	/// 終了処理(デストラクタでも呼ばれる).
	/// </summary>
	void Finalize();

	/// <summary>
	/// ノードの種類を登録する(同じ名前なら上書き).
	/// </summary>
	void RegisterNodeType(const NodeTypeDesc& desc);
	const std::map<std::string, NodeTypeDesc>& GetNodeTypes() const { return nodeTypes_; };

	/*=============================================================
	描画(Release版では何もしない)
	=============================================================*/
	/// <summary>
	/// name のウィンドウを開いてノードエディターを描画する.
	/// </summary>
	void Draw();

	/// <summary>
	/// 今開いているImGuiウィンドウの中にノードエディターを描画する.
	/// </summary>
	/// <param name="size">描画サイズ(0以下なら残りの領域いっぱい)</param>
	void DrawContent(const Vector2& size = { 0.0f, 0.0f });

	/// <summary>
	/// 全てのノードが見えるように表示位置を合わせる(次に描画した時に反映).
	/// </summary>
	void RequestNavigateToContent() { isNavigateRequested_ = true; };

	/*=============================================================
	グラフの操作
	=============================================================*/
	/// <summary>
	/// typeName の種類のノードを追加する.
	/// </summary>
	/// <returns>追加したノード(種類が登録されていなければnullptr)</returns>
	Node* AddNode(const std::string& typeName, const Vector2& position = { 0.0f, 0.0f });

	/// <summary>
	/// ノードと、そのノードに繋がっているリンクを削除する.
	/// </summary>
	void RemoveNode(uint32_t nodeId);

	/// <summary>
	/// 2つのピンを繋ぐ(順番はどちらでもよい). 入力ピンに既にリンクがあれば置き換える.
	/// </summary>
	/// <returns>追加したリンクのID(繋げない組み合わせなら0)</returns>
	uint32_t AddLink(uint32_t pinIdA, uint32_t pinIdB);

	void RemoveLink(uint32_t linkId);

	/// <summary>
	/// 2つのピンを繋げられるか(入力と出力 / 別のノード / 同じ型 / 分割中のピンではない).
	/// </summary>
	bool CanLink(const Pin& a, const Pin& b) const;

	/// <summary>
	/// 構造体のピン(Vector2 / Vector3 / Color)を要素ごとのfloatピンに分割する.
	/// 元のピンに繋がっていたリンクは外れる.
	/// </summary>
	/// <returns>分割した要素ピンを持つピン(分割できなければnullptr)</returns>
	Pin* SplitPin(uint32_t pinId);

	/// <summary>
	/// 分割をやめて1つのピンに戻す(pinIdは元のピンでも要素ピンでもよい). 要素ピンのリンクは外れる.
	/// </summary>
	bool UnsplitPin(uint32_t pinId);

	// ノードの位置を変える(キャンバス座標).
	void SetNodePosition(uint32_t nodeId, const Vector2& position);

	// 全てのノードとリンクを削除する(登録したノードの種類は残る).
	void Clear();

	/*=============================================================
	グラフの取得
	=============================================================*/
	const std::vector<std::unique_ptr<Node>>& GetNodes() const { return nodes_; };
	const std::vector<Link>& GetLinks() const { return links_; };

	Node* FindNode(uint32_t nodeId) const;
	// ピンを探す(分割した要素ピンも探す).
	Pin* FindPin(uint32_t pinId) const;
	const Link* FindLink(uint32_t linkId) const;

	// ピンにリンクが繋がっているか.
	bool IsPinLinked(uint32_t pinId) const;

	// 入力ピンに繋がっている出力ピン(繋がっていなければnullptr. 出力側が分割されていれば要素ピンが返る).
	Pin* GetLinkedOutputPin(uint32_t inputPinId) const;

	// 出力ピンに繋がっている入力ピン(複数あり得る).
	std::vector<Pin*> GetLinkedInputPins(uint32_t outputPinId) const;

	/// <summary>
	/// 入力ピンの値を取得する. 繋がっていれば繋がり先の出力ピンの値、なければ自分の値.
	/// 分割されていれば要素ピンの値をまとめた値を返す.
	/// </summary>
	PinValue GetInputValue(const Pin& inputPin) const;

	// GetInputValueを型を指定して取得する(型が違えばdefaultValue).
	template<typename T>
	T GetInputValueAs(const Pin& inputPin, const T& defaultValue = T{}) const {
		const PinValue value = GetInputValue(inputPin);
		const T* result = std::get_if<T>(&value);
		return result ? *result : defaultValue;
	}

	// エディターで選択されているノード(Release版では空).
	std::vector<uint32_t> GetSelectedNodeIds() const;

	// ノード / リンクの追加・削除、値・位置の変更があったか(呼ぶとフラグは下りる).
	bool ConsumeChanged();

	/*=============================================================
	保存 / 読み込み
	=============================================================*/
	/// <summary>
	/// グラフをJSONファイルに保存する.
	/// </summary>
	bool Save(const std::string& filePath) const;

	/// <summary>
	/// JSONファイルからグラフを読み込む(今のグラフは消える).
	/// 使われているノードの種類は先にRegisterNodeType()しておくこと. 登録されていない種類のノードは読み飛ばす.
	/// </summary>
	bool Load(const std::string& filePath);

	/*=============================================================
	値のユーティリティ
	=============================================================*/
	// PinTypeの既定値.
	static PinValue GetDefaultValue(PinType type);
	// 分割できる型か(Vector2 / Vector3 / Color).
	static bool CanSplit(PinType type) { return GetComponentCount(type) > 0; };
	// 構造体の要素数(分割できない型は0).
	static int GetComponentCount(PinType type);
	// 要素の名前("X" / "R" など).
	static const char* GetComponentName(PinType type, int index);
	// 構造体の値の要素を取得 / 設定する(構造体でなければ0 / 何もしない).
	static float GetComponent(const PinValue& value, int index);
	static void SetComponent(PinValue& value, int index, float component);

private:
	uint32_t GenerateId() { return nextId_++; };

	// ピンの構成を NodeTypeDesc から作る.
	void BuildPins(Node& node, const NodeTypeDesc& desc);

	// pin の要素ピンを作る(IDは新しく振る).
	void BuildSubPins(Pin& pin);

	// pinId(要素ピンも含む)に繋がるリンクを外す.
	void RemoveLinksOfPin(uint32_t pinId);

#ifdef USE_IMGUI
	void DrawNode(Node& node);
	void DrawInputPin(Pin& pin, const std::string& label);
	void DrawOutputPin(Pin& pin, const std::string& label, float labelWidth);
	void DrawInputValue(Pin& pin);
	void DrawPinIcon(const Pin& pin, bool isLinked) const;
	void UpdateCreate();
	void UpdateDelete();
	void DrawPopups();
	void DrawCreateNodeMenu(const std::string& categoryPrefix);
#endif // USE_IMGUI

private:
	std::string name_;
	ax::NodeEditor::EditorContext* context_ = nullptr;

	std::map<std::string, NodeTypeDesc> nodeTypes_;
	std::vector<std::unique_ptr<Node>> nodes_;
	std::vector<Link> links_;

	// 0は無効なIDとして使う.
	uint32_t nextId_ = 1;
	bool isChanged_ = false;
	bool isNavigateRequested_ = false;

	// ポップアップ用.
	uint32_t contextNodeId_ = 0;
	uint32_t contextLinkId_ = 0;
	uint32_t contextPinId_ = 0;
	// ピンからドラッグして何もない所で離した時の、そのピン(作ったノードと自動で繋ぐ).
	uint32_t newNodeLinkPinId_ = 0;
	bool isCreateMenuRequested_ = false;
	// ノードを作る位置(キャンバス座標).
	Vector2 newNodePosition_{};
	// 色を編集している入力ピン.
	uint32_t colorPickerPinId_ = 0;
	bool isColorPickerRequested_ = false;
};
