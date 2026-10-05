#include "Debug.h"
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>

Debug* Debug::GetInstance(){
	static Debug instance;
	return &instance;
}

void Debug::LoadDebugSettings(){

	const std::string filePath = "DebugSettings.ini";
	std::string line; // ファイルから読んだ1行を格納するもの.

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	if (!file.is_open()) {
		useDebugSettings = false;
		return;
	}

	useDebugSettings = true;

	while (getline(file, line)) {

		std::istringstream lineStream(line);

		std::string word;

		// データを取得.
		std::getline(lineStream, word, ' ');
	}

	// ファイルを閉じる
	file.close();
}
