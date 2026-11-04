#pragma once
#include <d3d12.h>
#include "WinApp.h"
#include "SrvManager.h"
#ifdef USE_IMGUI
#include "../../externals/imgui/imgui.h"
#include "../../externals/imgui/backends/imgui_impl_dx12.h"
#include "../../externals/imgui/backends/imgui_impl_win32.h"
#endif // USE_IMGUI
class ImGuiManager{
public:
	static ImGuiManager* GetInstance() { static ImGuiManager instance; return &instance; };

	void Initialize(WinApp* winApp,ID3D12Device* device,int32_t bufferCount,DXGI_FORMAT rtvDescFormat,ID3D12CommandQueue* commandQueue);
private:
};

