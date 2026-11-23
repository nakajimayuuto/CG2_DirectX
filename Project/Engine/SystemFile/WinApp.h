#pragma once
#include <Windows.h>
#include "../Environment/Environment.h"
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

	// ウィンドウ(クライアント領域)のサイズが変わっていたら、新しいサイズを受け取ってtrueを返す.
	// 一度受け取るとフラグは下がる.
	static bool ConsumeResize(uint32_t& width, uint32_t& height);
private:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
private:
	WNDCLASS wc{};

	HWND hwnd;

	// WM_SIZEで受け取ったクライアント領域のサイズ.
	static inline bool isResized_ = false;
	static inline uint32_t resizedWidth_ = 0;
	static inline uint32_t resizedHeight_ = 0;
};