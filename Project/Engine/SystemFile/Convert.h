#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>

class Convert{
public:

	// stringからwstringへ(配布)
	static std::wstring ConvertString(const std::string& str);

	// wstringからstringへ(配布)
	static std::string ConvertString(const std::wstring& str);
};

