#include "UpgradeManager.h"

#include <algorithm>
#include <cassert>
#include <format>

namespace {
	// 実行中に組み立てる文字列で使う文字(FontManagerがソース中の文字列から文字画像を作るため).
	[[maybe_unused]] const char* kUsedCharacters = "0123456789.,+-%/<> LvMAX財貨AB所持必要効果最大レベル説明";

	const char* GetGoldName(goldType type) {
		switch (type) {
		case goldType::kA: return "財貨A";
		case goldType::kB: return "財貨B";
		default:           return "";
		}
	}

	// 小数点以下が0なら整数で表示する.
	std::string FormatPercent(float percent) {
		if (percent == static_cast<float>(static_cast<int>(percent))) {
			return std::format("{}%", static_cast<int>(percent));
		}
		return std::format("{:.1f}%", percent);
	}
}

UpgradeManager* UpgradeManager::GetInstance() {
	static UpgradeManager instance;
	return &instance;
}

/*=============================================================
初期化.
=============================================================*/
void UpgradeManager::Initialize() {
	// 項目, 名前, 説明, 財貨, 最大レベル, 1レベルごとの%, 必要財貨(Lv0→1), 1レベルごとの必要財貨の増加.
	CreateData(upgradeName::kAttack, "攻撃力", "与えるダメージが\n増える", goldType::kA, 5, 10.0f, 100, 50);
	CreateData(upgradeName::kAttackSpeed, "攻撃速度", "攻撃の間隔が\n短くなる", goldType::kA, 5, 8.0f, 120, 60);
	CreateData(upgradeName::kMoveSpeed, "移動速度", "移動が速くなる", goldType::kA, 3, 10.0f, 80, 40);

	// レベルごとに直接指定する版 : 項目, 名前, 説明, 財貨, { Lv1,Lv2,... の% }, { Lv0→1,Lv1→2,... の必要財貨 }.
	CreateData(upgradeName::kMaxHP, "最大HP", "最大HPが増える", goldType::kB, { 10.0f,25.0f,50.0f }, { 1,2,4 });
	CreateData(upgradeName::kDefense, "防御力", "受けるダメージが\n減る", goldType::kB, 5, 5.0f, 1, 1);
	CreateData(upgradeName::kGoldBonus, "財貨獲得", "手に入る財貨が\n増える", goldType::kB, 3, 20.0f, 2, 2);

#ifdef _DEBUG
	// 作り忘れチェック.
	for (const UpgradeData& data : datas_) {
		assert(data.isCreated && "UpgradeManager : CreateDataしていない項目があります");
	}
#endif

	DataClear();
}

void UpgradeManager::DataClear() {
	for (UpgradeData& data : datas_) {
		data.level = 0;
	}
	golds_.fill(0);

	for (auto& row : choices_) {
		row.clear();
	}
	cursorRow_ = 0;
	cursorColumn_ = 0;
	isActive_ = false;
}

void UpgradeManager::CreateData(upgradeName upgrade, const std::string& name, const std::string& description, goldType type,
	int maxLevel, float percentPerLevel, int baseCost, int costPerLevel) {
	std::vector<float> percents;
	std::vector<int> costs;
	for (int level = 1; level <= maxLevel; level++) {
		percents.push_back(percentPerLevel * static_cast<float>(level));
		costs.push_back(baseCost + costPerLevel * (level - 1));
	}
	CreateData(upgrade, name, description, type, percents, costs);
}

void UpgradeManager::CreateData(upgradeName upgrade, const std::string& name, const std::string& description, goldType type,
	const std::vector<float>& percents, const std::vector<int>& costs) {
	assert(upgrade != upgradeName::kCount && type != goldType::kCount);
	assert(percents.size() == costs.size() && "UpgradeManager : percentsとcostsの要素数が違います");

	UpgradeData& data = datas_[Index(upgrade)];
	data.name = name;
	data.description = description;
	data.type = type;
	data.level = 0;
	data.maxLevel = static_cast<int>((std::min)(percents.size(), costs.size()));

	data.percents.clear();
	data.percents.push_back(0.0f); // レベル0.
	data.percents.insert(data.percents.end(), percents.begin(), percents.begin() + data.maxLevel);
	data.costs.assign(costs.begin(), costs.begin() + data.maxLevel);

	data.isCreated = true;
}

/*=============================================================
取得・財貨.
=============================================================*/
float UpgradeManager::GetPercent(upgradeName upgrade) const {
	const UpgradeData& data = GetData(upgrade);
	if (data.percents.empty()) {
		return 0.0f;
	}
	return data.percents[std::clamp(data.level, 0, data.maxLevel)];
}

int UpgradeManager::GetCost(upgradeName upgrade) const {
	const UpgradeData& data = GetData(upgrade);
	if (data.level >= data.maxLevel) {
		return -1;
	}
	return data.costs[data.level];
}

void UpgradeManager::AddGold(goldType type, int amount) {
	int& gold = golds_[Index(type)];
	gold = (std::max)(gold + amount, 0);
}

bool UpgradeManager::LevelUp(upgradeName upgrade) {
	UpgradeData& data = datas_[Index(upgrade)];
	if (!data.isCreated || data.level >= data.maxLevel) {
		return false;
	}
	const int cost = data.costs[data.level];
	int& gold = golds_[Index(data.type)];
	if (gold < cost) {
		return false;
	}
	gold -= cost;
	data.level++;
	return true;
}

/*=============================================================
選択画面.
=============================================================*/
void UpgradeManager::ChangeUIActive() {
	isActive_ = !isActive_;
	if (!isActive_) {
		return;
	}

	// 開いた時に並べる項目を選び直して、カーソルを項目のある最初の段に置く.
	RollChoices();
	cursorRow_ = 0;
	cursorColumn_ = 0;
	for (size_t row = 0; row < kRowNum_; row++) {
		if (!choices_[row].empty()) {
			cursorRow_ = static_cast<int>(row);
			break;
		}
	}
}

void UpgradeManager::RollChoices() {
	for (size_t row = 0; row < kRowNum_; row++) {
		std::vector<upgradeName>& choice = choices_[row];
		choice.clear();

		// その財貨で強化でき、まだ最大レベルではない項目から選ぶ.
		for (size_t i = 0; i < datas_.size(); i++) {
			const UpgradeData& data = datas_[i];
			if (data.isCreated && Index(data.type) == row && data.level < data.maxLevel) {
				choice.push_back(static_cast<upgradeName>(i));
			}
		}
		std::shuffle(choice.begin(), choice.end(), randomEngine_);
		if (choice.size() > static_cast<size_t>(kColumnNum_)) {
			choice.resize(kColumnNum_);
		}
	}
}

upgradeName UpgradeManager::GetSelected() const {
	if (cursorRow_ < 0 || cursorRow_ >= static_cast<int>(kRowNum_)) {
		return upgradeName::kCount;
	}
	const std::vector<upgradeName>& row = choices_[cursorRow_];
	if (cursorColumn_ < 0 || cursorColumn_ >= static_cast<int>(row.size())) {
		return upgradeName::kCount;
	}
	return row[cursorColumn_];
}

void UpgradeManager::Update() {
	if (!isActive_) {
		return;
	}
	InputManager* input = InputManager::GetInstance();

	// 左右 : 同じ段の中で移動(端で止まる).
	const int columnCount = static_cast<int>(choices_[cursorRow_].size());
	if (input->TriggerKey(DIK_LEFT) && cursorColumn_ > 0) {
		cursorColumn_--;
	}
	if (input->TriggerKey(DIK_RIGHT) && cursorColumn_ < columnCount - 1) {
		cursorColumn_++;
	}

	// 上下 : 項目のある段へ移動(列は移動先の項目数に合わせる).
	auto moveRow = [&](int direction) {
		for (int row = cursorRow_ + direction; 0 <= row && row < static_cast<int>(kRowNum_); row += direction) {
			if (!choices_[row].empty()) {
				cursorRow_ = row;
				cursorColumn_ = (std::min)(cursorColumn_, static_cast<int>(choices_[row].size()) - 1);
				return;
			}
		}
	};
	if (input->TriggerKey(DIK_UP)) {
		moveRow(-1);
	}
	if (input->TriggerKey(DIK_DOWN)) {
		moveRow(1);
	}

	// Space : 選択中の項目のレベルを1上げる.
	if (input->TriggerKey(DIK_SPACE)) {
		upgradeName selected = GetSelected();
		if (selected != upgradeName::kCount) {
			LevelUp(selected);
		}
	}
}

void UpgradeManager::Draw() {
	if (!isActive_) {
		return;
	}
	DialogBoxManager* dialog = DialogBoxManager::GetInstance();
	dialog->SetFontSize(fontSize_);

	// UI全体の大きさから、uiCenter_が中心になるように項目の左上を求める.
	const float gridWidth = (cellSize_.x + cellSpace_) * static_cast<float>(kColumnNum_) - cellSpace_;
	const float gridHeight = (cellSize_.y + cellSpace_) * static_cast<float>(kRowNum_) - cellSpace_;
	const float totalWidth = gridWidth + descriptionSpace_ + descriptionWidth_;
	const Vector2 gridPosition = { uiCenter_.x - totalWidth * 0.5f, uiCenter_.y - gridHeight * 0.5f };

	/*=============================================================
	強化項目(上の段 : 財貨A、下の段 : 財貨B).
	=============================================================*/
	// 2行(名前 + 必要財貨)が縦の真ん中に来るように余白を決める.
	const float lineHeight = fontSize_ * 1.25f;
	dialog->SetTextAlign(TextAlign::kCenter);
	dialog->SetTextPadding({ 0.0f, (cellSize_.y - lineHeight * 2.0f) * 0.5f });

	const upgradeName selected = GetSelected();
	for (size_t row = 0; row < kRowNum_; row++) {
		for (size_t column = 0; column < choices_[row].size(); column++) {
			const upgradeName upgrade = choices_[row][column];
			Vector2 position = {
				gridPosition.x + (cellSize_.x + cellSpace_) * static_cast<float>(column),
				gridPosition.y + (cellSize_.y + cellSpace_) * static_cast<float>(row) };

			const int cost = GetCost(upgrade);
			std::string text = GetName(upgrade) + "\n" + (cost < 0 ? std::string("MAX") : std::to_string(cost));

			const Vector4& textColor = (upgrade == selected) ? selectColor_ : normalColor_;
			dialog->DrawDialogBox(position, cellSize_, text, textColor, backgroundColor_);
		}
	}

	/*=============================================================
	説明(選択中の項目).
	=============================================================*/
	dialog->SetTextAlign(TextAlign::kLeft);
	dialog->SetTextPadding({ 16.0f,16.0f });

	Vector2 descriptionPosition = { gridPosition.x + gridWidth + descriptionSpace_, gridPosition.y };
	Vector2 descriptionSize = { descriptionWidth_, gridHeight };

	std::string description;
	if (selected != upgradeName::kCount) {
		const UpgradeData& data = GetData(selected);
		description += data.name + "\n";
		description += std::format("Lv {} / {}\n", data.level, data.maxLevel);
		if (data.level < data.maxLevel) {
			description += "効果 +" + FormatPercent(GetPercent(selected)) + " -> +" + FormatPercent(data.percents[data.level + 1]) + "\n";
			description += std::format("必要 {} {}\n", GetGoldName(data.type), GetCost(selected));
		} else {
			description += "効果 +" + FormatPercent(GetPercent(selected)) + " (MAX)\n";
		}
		description += data.description + "\n";
	}
	description += std::format("所持 {} {}  {} {}",
		GetGoldName(goldType::kA), GetGold(goldType::kA),
		GetGoldName(goldType::kB), GetGold(goldType::kB));

	dialog->DrawDialogBox(descriptionPosition, descriptionSize, description, normalColor_, backgroundColor_);
}
