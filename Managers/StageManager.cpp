#include "StageManager.h"
#include <map>
#include <assert.h>
#include <fstream>
#include <sstream>

StageManager* StageManager::GetInstance() {
	static StageManager instance;
	return &instance;
}

void StageManager::LoadStageDataFile() {

	const std::string filePath = "Resources/map/stageDatas.csv";

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	// csv
	std::stringstream stageDataCsv;
	// 文字列をコピー.
	stageDataCsv << file.rdbuf();

	// ファイルを閉じる
	file.close();

	std::string line;
	while (getline(stageDataCsv, line)) {

		std::istringstream lineStream(line);

		// ステージデータを格納する構造体.
		StageData stageData;

		// カンマ区切りの一つ分を格納するstring.
		std::string word;

		// データを取得.
		std::getline(lineStream, word, ',');

		if (word == "") {
			continue;
		}

		stageData.name = word;

		// データを取得.
		std::getline(lineStream, word, ',');

		stageData.timeLimit = std::stoi(word);

		stageDatas_.push_back(stageData);
	}
}

void StageManager::SetCurrentStageIndexByName(const std::string& name){
	//auto index = std::find_if(stageDatas_.begin(), stageDatas_.end(),name);
	for (size_t i = 0; i < stageDatas_.size(); i++) {
		if (stageDatas_[i].name == name) {
			currentStageIndex_ = static_cast<int32_t>(i);
			return;
		}
	}

	assert(false && "指定されたステージ名は存在しません");
}
