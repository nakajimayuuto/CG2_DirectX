#pragma once
#include <vector>
#include <algorithm>

class ContactRecord{
public:
	void Clear() { record.clear(); };

	void AddRecord(uint32_t number) { record.push_back(number); };

	bool RecordCheck(uint32_t number) { return std::any_of(record.begin(), record.end(), [number](uint32_t index) { return index == number; }); };
private:
	std::vector<uint32_t> record;
};

