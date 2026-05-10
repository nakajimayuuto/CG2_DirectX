#include "GlobalVariables.h"
#include "./Engine/SystemFile/ImGui.h"
#include <fstream>
#include <sstream>
#include <Windows.h>

GlobalVariables* GlobalVariables::GetInstance() {
	static GlobalVariables instance;
	return &instance;
}

void GlobalVariables::Update() {
	if (!ImGui::Begin("GlobalVariables", nullptr, ImGuiWindowFlags_MenuBar)) {
		ImGui::End();
		return;
	}

	if (!ImGui::BeginMenuBar()) return;

	for (std::map<std::string, Group>::iterator itGroup = datas_.begin();
		itGroup != datas_.end(); itGroup++) {
		// グループ名を取得.
		const std::string& groupName = itGroup->first;
		// グループの参照を取得.
		Group& group = itGroup->second;

		if (!ImGui::BeginMenu(groupName.c_str())) continue;

		for (std::map<std::string,Item>::iterator itItem = group.begin();
			itItem != group.end(); itItem++) {
			// 項目名を取得.
			const std::string& itemName = itItem->first;
			// 項目の参照を取得.
			Item& item = itItem->second;

			if (std::holds_alternative<int32_t>(item)) {
				int32_t* ptr = std::get_if<int32_t>(&item);
				ImGui::SliderInt(itemName.c_str(),ptr,0,100);
			} else if (std::holds_alternative<float>(item)) {
				float* ptr = std::get_if<float>(&item);
				ImGui::SliderFloat(itemName.c_str(), ptr, -10.0f, 10.0f);
			} else if (std::holds_alternative<Vector3>(item)) {
				Vector3* ptr = std::get_if<Vector3>(&item);
				ImGui::SliderFloat3(itemName.c_str(), reinterpret_cast<float*>(ptr), 0, 100);
			}
		}

		ImGui::Text("\n");

		if (ImGui::Button("Save")) {
			SaveFile(groupName);
			std::string message = std::format("{}.json saved",groupName);
			MessageBoxA(nullptr,message.c_str(),"GlobalVariabes",0);
		}

		ImGui::EndMenu();
	}

	ImGui::EndMenuBar();
	ImGui::End();
}

void GlobalVariables::CreateGroup(const std::string& groupName) {
	datas_[groupName];
}

void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, int32_t value) {
	Group& group = datas_[groupName];

	Item newItem{};
	newItem = value;
	group[key] = newItem;
}

void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, float value) {
	Group& group = datas_[groupName];

	Item newItem{};
	newItem = value;
	group[key] = newItem;
}

void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, const Vector3& value) {
	Group& group = datas_[groupName];

	Item newItem{};
	newItem = value;
	group[key] = newItem;
}

void GlobalVariables::AddValue(const std::string& groupName, const std::string& key, int32_t value){
	Group& group = datas_[groupName];

	std::map<std::string, Item>::iterator itItem = group.find(key);

	// 未登録チェック.
	if (itItem != group.end()) {
		return;
	}

	SetValue(groupName,key,value);
}

void GlobalVariables::AddValue(const std::string& groupName, const std::string& key, float value){
	Group& group = datas_[groupName];

	std::map<std::string, Item>::iterator itItem = group.find(key);

	// 未登録チェック.
	if (itItem != group.end()) {
		return;
	}

	SetValue(groupName, key, value);
}

void GlobalVariables::AddValue(const std::string& groupName, const std::string& key, const Vector3& value){
	Group& group = datas_[groupName];

	std::map<std::string, Item>::iterator itItem = group.find(key);

	// 未登録チェック.
	if (itItem != group.end()) {
		return;
	}

	SetValue(groupName, key, value);
}

int32_t GlobalVariables::GetIntValue(const std::string& groupName, const std::string& key){
	// グループを検索.
	std::map<std::string, Group>::iterator itGroup = datas_.find(groupName);

	// 未登録チェック.
	assert(itGroup != datas_.end());

	Group& group = datas_.at(groupName);

	std::map<std::string, Item>::iterator itItem = group.find(key);

	// 未登録チェック.
	assert(itItem != group.end());
	
	Item& item = itItem->second;

	return std::get<int32_t>(item);
}

float GlobalVariables::GetFloatValue(const std::string& groupName, const std::string& key){
	// グループを検索.
	std::map<std::string, Group>::iterator itGroup = datas_.find(groupName);

	// 未登録チェック.
	assert(itGroup != datas_.end());

	Group& group = datas_.at(groupName);

	std::map<std::string, Item>::iterator itItem = group.find(key);

	// 未登録チェック.
	assert(itItem != group.end());

	Item& item = itItem->second;

	return std::get<float>(item);
}

Vector3 GlobalVariables::GetVector3Value(const std::string& groupName, const std::string& key){
	// グループを検索.
	std::map<std::string, Group>::iterator itGroup = datas_.find(groupName);

	// 未登録チェック.
	assert(itGroup != datas_.end());

	Group& group = datas_.at(groupName);

	std::map<std::string, Item>::iterator itItem = group.find(key);

	// 未登録チェック.
	assert(itItem != group.end());

	Item& item = itItem->second;

	return std::get<Vector3>(item);
}

void GlobalVariables::SaveFile(const std::string& groupName){
	// グループを検索.
	std::map<std::string, Group>::iterator itGroup = datas_.find(groupName);

	// 未登録チェック.
	assert(itGroup != datas_.end());

	json root;
	root = json::object();

	root[groupName] = json::object();

	for (std::map<std::string, Item>::iterator itItem = itGroup->second.begin();
		itItem != itGroup->second.end();itItem++) {

		// 項目名を取得.
		const std::string& itemName = itItem->first;
		// 項目の参照を取得.
		Item& item = itItem->second;

		if (std::holds_alternative<int32_t>(item)) {
			root[groupName][itemName] = std::get<int32_t>(item);
		} else if (std::holds_alternative<float>(item)) {
			root[groupName][itemName] = std::get<float>(item);
		} else if (std::holds_alternative<Vector3>(item)) {
			Vector3 value = std::get<Vector3>(item);
			root[groupName][itemName] = json::array({value.x,value.y,value.z});
		}

	}

	// ディレクトリがなければ作成する.
	std::filesystem::path dir(kDirectoryPath);
	if (!std::filesystem::exists(kDirectoryPath)) {
		std::filesystem::create_directory(kDirectoryPath);
	}

	// 書き込むJSONファイルのフルパスを合成する.
	std::string filePath = kDirectoryPath + groupName + ".json";

	// 書き込む用ファイルストリーム.
	std::ofstream ofs;

	ofs.open(filePath);

	if (ofs.fail()) {
		std::string message = "Failed open data file for write";
		MessageBoxA(nullptr,message.c_str(),"GlobalVariable",0);
		assert(0);
		return;
	}

	// ファイルにjson文字列を書き込む(インデント幅4).
	ofs << std::setw(4) << root << std::endl;

	// ファイルを閉じる.
	ofs.close();
}

void GlobalVariables::LoadFiles(){
	// ディレクトリがなければスキップする.
	std::filesystem::path dir(kDirectoryPath);
	if (!std::filesystem::exists(kDirectoryPath)) {
		return;
	}

	std::filesystem::directory_iterator dir_it(kDirectoryPath);
	for (const std::filesystem::directory_entry& entry : dir_it) {
		// ファイルパスを取得.
		const std::filesystem::path& filePath = entry.path();

		// ファイル拡張子を取得.
		std::string extension = filePath.extension().string();
		// .jsonファイル以外はスキップ.
		if (extension.compare(".json") != 0) {
			continue;
		}
		
		LoadFile(filePath.stem().string());
	}

}

void GlobalVariables::LoadFile(const std::string& groupName){
	// 読み込むJSONファイルのフルパスを合成する.
	std::string filePath = kDirectoryPath + groupName + ".json";
	// 読み込む用ファイルストリーム.
	std::ifstream ifs;
	// ファイルを読み込み用に開く.
	ifs.open(filePath);

	if (ifs.fail()) {
		std::string message = "Failed open data file for read";
		MessageBoxA(nullptr, message.c_str(), "GlobalVariable", 0);
		assert(0);
		return;
	}

	json root;

	// json文字列からデータ構造に展開.
	ifs >> root;
	// ファイルを閉じる.
	ifs.close();

	// グループを検索.
	json::iterator itGroup = root.find(groupName);

	// 未登録チェック.
	assert(itGroup != root.end());

	for (json::iterator itItem = itGroup->begin(); itItem != itGroup->end();itItem++) {
		// アイテム名を取得.
		const std::string& itemName = itItem.key();

		if (itItem->is_number_integer()) {
			int32_t value = itItem->get<int32_t>();
			SetValue(groupName,itemName,value);
		}else if(itItem->is_number_float()) {
			float value = itItem->get<float>();
			SetValue(groupName, itemName, value);
		} else if (itItem->is_array() && itItem->size() == 3) {
			Vector3 value = {itItem->at(0),itItem->at(1) ,itItem->at(2) };
			SetValue(groupName, itemName, value);
		}

	}

}
