#pragma once
#include "./Engine/Math/Vector3.h"
#include <variant>
#include <map>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

class GlobalVariables final{
public:
	GlobalVariables& operator=(const GlobalVariables& obj) = delete;
	GlobalVariables(const GlobalVariables& obj) = delete;

	static GlobalVariables* GetInstance();

	void Update();


	void CreateGroup(const std::string& groupName);
	void SetValue(const std::string& groupName, const std::string& key,int32_t value);
	void SetValue(const std::string& groupName, const std::string& key,float value);
	void SetValue(const std::string& groupName, const std::string& key,const Vector3& value);

	/// <summary>
	/// ファイルに書き出し.
	/// </summary>
	/// <param name="groupName">グループ名</param>
	void SaveFile(const std::string& groupName);


private:
	GlobalVariables() = default;
	~GlobalVariables() = default;
private:
	const std::string kDirectoryPath = "Resource/GlobalVariables/";

	struct Item {
		std::variant<int32_t, float, Vector3> value;
	};

	struct Group {
		std::map<std::string, Item> item;
	};

	std::map<std::string, Group> datas_;
};

