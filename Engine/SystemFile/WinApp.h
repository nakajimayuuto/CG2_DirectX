#pragma once
#include <Windows.h>
#include "../../Environment.h"
#include <wrl.h>

class WinApp {
public:
	void Initialize();

	void Update();
private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
private:
	
};