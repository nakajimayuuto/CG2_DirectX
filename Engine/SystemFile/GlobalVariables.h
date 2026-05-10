#pragma once
#include "../Math/Vector3.h"
#include <variant>
#include <map>
#include <string>
#include "../../externals/nlohmann/json.hpp"

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

	void AddValue(const std::string& groupName, const std::string& key,int32_t value);
	void AddValue(const std::string& groupName, const std::string& key,float value);
	void AddValue(const std::string& groupName, const std::string& key,const Vector3& value);

	int32_t GetIntValue(const std::string& groupName, const std::string& key);
	float GetFloatValue(const std::string& groupName, const std::string& key);
	Vector3 GetVector3Value(const std::string& groupName, const std::string& key);

	/// <summary>
	/// ファイルに書き出し.
	/// </summary>
	/// <param name="groupName">グループ名</param>
	void SaveFile(const std::string& groupName);

	/// <summary>
	/// ディレクトリの全ファイル読み込み.
	/// </summary>
	void LoadFiles();

	/// <summary>
	/// ファイルから読み込む.
	/// </summary>
	/// <param name="groupName">グループ</param>
	void LoadFile(const std::string& groupName);
private:
	GlobalVariables() = default;
	~GlobalVariables() = default;
private:
	const std::string kDirectoryPath = "Resource/GlobalVariables/";

	using Item = std::variant<int32_t, float, Vector3>;

	using Group = std::map<std::string, Item>;

	std::map<std::string, Group> datas_;
};

