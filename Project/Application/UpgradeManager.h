#pragma once
#include <array>
#include <string>
#include <vector>
#include <random>
#include "Satlib.h"

/// <summary>
/// 強化項目の名前(この値で強化数値を管理する).
/// 項目を増やす時はkCountより前に追加して、UpgradeManager::InitializeでCreateDataを呼ぶ.
/// </summary>
enum class upgradeName {
	kAttack,      // 攻撃力.
	kAttackSpeed, // 攻撃速度.
	kMoveSpeed,   // 移動速度.
	kMaxHP,       // 最大HP.
	kDefense,     // 防御力.
	kGoldBonus,   // 財貨獲得量.

	kCount,
};

/// <summary>
/// 財貨のタイプ.
/// </summary>
enum class goldType {
	kA, // 財貨A(選択画面の上の段).
	kB, // 財貨B(選択画面の下の段).

	kCount,
};

/// <summary>
/// ローグライクの強化数値の管理.
/// </summary>
class UpgradeManager {
public:
	/// <summary>
	/// 1項目分のデータ.
	/// </summary>
	struct UpgradeData {
		std::string name;              // 項目の名前.
		std::string description;       // 説明.
		goldType type = goldType::kA;  // 強化に使う財貨のタイプ.
		int level = 0;                 // 現在のレベル.
		int maxLevel = 0;              // 最大レベル.
		std::vector<float> percents;   // レベルごとの数値の%(要素数 = maxLevel + 1、[0]はレベル0).
		std::vector<int> costs;        // レベルごとの強化に必要な財貨(要素数 = maxLevel、[n]はレベルn→n+1).
		bool isCreated = false;
	};

public:
	static UpgradeManager* GetInstance();

	// 項目のデータを作る(ApplicationのLoadDatasの後で一度だけ呼ぶ).
	void Initialize();
	// レベル・所持財貨・選択状態をリセットする(ゲーム開始時).
	void DataClear();
	// 選択画面の操作(isActive_がtrueの時だけ).
	void Update();
	// 選択画面の描画(isActive_がtrueの時だけ).
	void Draw();

	// 選択画面の表示/非表示を切り替える.
	void ChangeUIActive();
	bool GetIsActive() const { return isActive_; };

	/// <summary>
	/// 項目のデータを作る(1レベルごとに一定量ずつ増える版).
	/// </summary>
	/// <param name="upgrade">項目</param>
	/// <param name="name">選択画面に出す名前</param>
	/// <param name="description">説明("\n"で改行)</param>
	/// <param name="type">強化に使う財貨のタイプ</param>
	/// <param name="maxLevel">最大レベル</param>
	/// <param name="percentPerLevel">1レベルごとに増える数値(%)</param>
	/// <param name="baseCost">レベル0→1に必要な財貨</param>
	/// <param name="costPerLevel">1レベルごとに増える必要財貨</param>
	void CreateData(upgradeName upgrade, const std::string& name, const std::string& description, goldType type,
		int maxLevel, float percentPerLevel, int baseCost, int costPerLevel);

	/// <summary>
	/// 項目のデータを作る(レベルごとに数値を直接指定する版).
	/// </summary>
	/// <param name="percents">レベル1,2,3...の時の数値(%)。要素数が最大レベルになる</param>
	/// <param name="costs">レベル0→1,1→2...に必要な財貨。percentsと同じ要素数</param>
	void CreateData(upgradeName upgrade, const std::string& name, const std::string& description, goldType type,
		const std::vector<float>& percents, const std::vector<int>& costs);

	/*=============================================================
	取得.
	=============================================================*/
	const UpgradeData& GetData(upgradeName upgrade) const { return datas_[Index(upgrade)]; };
	const std::string& GetName(upgradeName upgrade) const { return GetData(upgrade).name; };
	const std::string& GetDescription(upgradeName upgrade) const { return GetData(upgrade).description; };
	int GetLevel(upgradeName upgrade) const { return GetData(upgrade).level; };
	int GetMaxLevel(upgradeName upgrade) const { return GetData(upgrade).maxLevel; };
	bool IsMaxLevel(upgradeName upgrade) const { return GetLevel(upgrade) >= GetMaxLevel(upgrade); };
	// 現在のレベルの数値(%). 例 : 20.0f → +20%.
	float GetPercent(upgradeName upgrade) const;
	// 現在のレベルの倍率. 例 : +20% → 1.2f.
	float GetRate(upgradeName upgrade) const { return 1.0f + GetPercent(upgrade) / 100.0f; };
	// 次のレベルに上げるのに必要な財貨(最大レベルなら-1).
	int GetCost(upgradeName upgrade) const;

	/*=============================================================
	財貨.
	=============================================================*/
	int GetGold(goldType type) const { return golds_[Index(type)]; };
	void AddGold(goldType type, int amount);

	// 財貨を使ってレベルを1上げる(足りない/最大レベルならfalse).
	bool LevelUp(upgradeName upgrade);

private:
	UpgradeManager() = default;
	~UpgradeManager() = default;
	UpgradeManager(const UpgradeManager&) = delete;
	UpgradeManager& operator=(const UpgradeManager&) = delete;

	static size_t Index(upgradeName upgrade) { return static_cast<size_t>(upgrade); };
	static size_t Index(goldType type) { return static_cast<size_t>(type); };

	// 選択画面に出す項目を財貨のタイプごとに選び直す.
	void RollChoices();
	// 選択中の項目(無ければkCount).
	upgradeName GetSelected() const;

private:
	static inline const int kColumnNum_ = 3; // 1段に並べる項目の数.
	static inline const size_t kRowNum_ = static_cast<size_t>(goldType::kCount);

	std::array<UpgradeData, static_cast<size_t>(upgradeName::kCount)> datas_{};
	std::array<int, static_cast<size_t>(goldType::kCount)> golds_{};

	// 選択画面に並んでいる項目([段][列]、段 = 財貨のタイプ).
	std::array<std::vector<upgradeName>, kRowNum_> choices_{};
	int cursorRow_ = 0;
	int cursorColumn_ = 0;

	bool isActive_ = false;

	std::mt19937 randomEngine_{ std::random_device{}() };

	/*=============================================================
	レイアウト(UI座標 : 画面の中心が(0,0)、右が+X、下が+Y、px).
	=============================================================*/
	Vector2 uiCenter_ = { 0.0f,0.0f };           // UI全体(項目 + 説明)の中心.
	Vector2 cellSize_ = { 130.0f,100.0f };       // 項目1つの大きさ.
	float cellSpace_ = 20.0f;                    // 項目同士の間隔.
	float descriptionSpace_ = 30.0f;             // 項目と説明の間隔.
	float descriptionWidth_ = 300.0f;            // 説明の幅.
	float fontSize_ = 18.0f;

	Vector4 selectColor_ = { 1.0f,1.0f,0.0f,1.0f }; // 選択中の文字の色(黄).
	Vector4 normalColor_ = { 1.0f,1.0f,1.0f,1.0f }; // それ以外の文字の色(白).
	Vector4 backgroundColor_ = { 0.0f,0.0f,0.0f,1.0f };
};
