#include "GlobalVariables.h"
#include "./Engine/SystemFile/ImGui.h"
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
