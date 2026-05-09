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

		for (std::map<std::string,Item>::iterator itItem = group.item.begin();
			itItem != group.item.end(); itItem++) {
			// 項目名を取得.
			const std::string& itemName = itItem->first;
			// 項目の参照を取得.
			Item& item = itItem->second;

			if (std::holds_alternative<int32_t>(item.value)) {
				int32_t* ptr = std::get_if<int32_t>(&item.value);
				ImGui::SliderInt(itemName.c_str(),ptr,0,100);
			} else if (std::holds_alternative<float>(item.value)) {
				float* ptr = std::get_if<float>(&item.value);
				ImGui::SliderFloat(itemName.c_str(), ptr, -10.0f, 10.0f);
			} else if (std::holds_alternative<Vector3>(item.value)) {
				Vector3* ptr = std::get_if<Vector3>(&item.value);
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
	newItem.value = value;
	group.item[key] = newItem;
}

void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, float value) {
	Group& group = datas_[groupName];

	Item newItem{};
	newItem.value = value;
	group.item[key] = newItem;
}

void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, const Vector3& value) {
	Group& group = datas_[groupName];

	Item newItem{};
	newItem.value = value;
	group.item[key] = newItem;
}

void GlobalVariables::SaveFile(const std::string& groupName){
	// グループを検索.
	std::map<std::string, Group>::iterator itGroup = datas_.find(groupName);

	// 未登録チェック.
	assert(itGroup != datas_.end());

	json root;
	root = json::object();

	root[groupName] = json::object();

	for (std::map<std::string, Item>::iterator itItem = itGroup->second.item.begin();
		itItem != itGroup->second.item.end();itItem++) {

		// 項目名を取得.
		const std::string& itemName = itItem->first;
		// 項目の参照を取得.
		Item& item = itItem->second;

		if (std::holds_alternative<int32_t>(item.value)) {
			root[groupName][itemName] = std::get<int32_t>(item.value);
		} else if (std::holds_alternative<float>(item.value)) {
			root[groupName][itemName] = std::get<float>(item.value);
		} else if (std::holds_alternative<Vector3>(item.value)) {
			Vector3 value = std::get<Vector3>(item.value);
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
