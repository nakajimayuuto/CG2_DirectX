#pragma once
class Debug{
public:
	static Debug* GetInstance();

	void LoadDebugSettings();
private:
	bool useDebugSettings = false;
};

