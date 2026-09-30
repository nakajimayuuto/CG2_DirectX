#pragma once
#pragma comment(lib,"winmm.lib")
#include <Windows.h>
#include "../../Environment.h"
#include <wrl.h>
#include <vector>

#ifdef USE_IMGUI
#include "../../externals/imgui/imgui.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI
class WinApp {
public:
	void Initialize();

	void Update();

	void Finalize();

	HWND GetHWND() const { return hwnd; };
	HINSTANCE GetHInstance() const{ return wc.hInstance; };

	bool ProcessMessage();
private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
private:
	WNDCLASS wc{};

	HWND hwnd;
};