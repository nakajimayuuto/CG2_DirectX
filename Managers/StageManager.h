#pragma once
#include "../Satlib.h"
#include <vector>
struct StageData {
	std::string name; // ステージ名.
	int32_t timeLimit; // 制限時間.
};

class StageManager{
public:

	static StageManager* GetInstance();

	void LoadStageDataFile();

	/// <summary>
	/// ステージデータの取得
	/// </summary>
	/// <param name="index">ステージ番号</param>
	/// <returns>ステージデータ</returns>
	const StageData& GetStageData(int32_t index) const {
		assert(index <= stageDatas_.size() && "ステージデータの範囲外です");
		return stageDatas_[index];
	}

	void SetCurrentStageIndex(int32_t index) {
		assert(index <= stageDatas_.size() && "ステージデータの範囲外です");
		currentStageIndex_ = index;
	}

	int32_t GetCurrentStageIndex() { return currentStageIndex_; };

	/// <summary>
	/// 現在のステージデータの取得
	/// </summary>
	/// <returns>ステージデータ</returns>
	const StageData& GetCurrentStageData() const {
		return GetStageData(currentStageIndex_);
	}

	/// <summary>
	/// ステージ名で現在のステージ番号設定
	/// </summary>
	/// <param name="name">ステージ名</param>
	void SetCurrentStageIndexByName(const std::string& name);

private:
	std::vector<StageData> stageDatas_;
	
	int32_t currentStageIndex_ = 1;

};

