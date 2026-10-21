#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <dbghelp.h>
#include <vector>
#include <numbers>
#include "WinApp.h"
#include "Pipeline.h"


#include "../Math/Vector4.h"
#include "../Math/Vertex.h"
#include "../Math/Matrix4x4.h"
#include "../Math/Transform.h"
#include "../Math/MyMath.h"
#include "../Environment/Environment.h"

#include "../Renderer/Material.h"
#include "../Renderer/DirectionalLight.h"
#include "Convert.h"

#include "../Renderer/ModelData.h"
#include <fstream>
#include <sstream>

#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"

#ifdef USE_IMGUI
#include "../../externals/imgui/imgui.h"
#include "../../externals/imgui/imgui_impl_dx12.h"
#include "../../externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI
struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker()
	{
		// リソースリークチェック.
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			//debug->Release();
		}
	}
};

struct ModelElement;

class GameSystem {
private:
public:
	static GameSystem* GetInstance();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	void Initialize();

	bool ProcessMessage();

	bool BeginFrame();

	void DrawSetup();

	void EndFrame();

	void Finalize();
private:
	void DXFactoryInitialize();

	void CommandInitialize();

	void SwapChainInitialize();

	void DescriptorInitialize();

	void RenderTargetViewInitialize();

	void FenceInitialize();

	void ViewportInitialize();

	void ScissorInitialize();

	void PipelineInitialize();
	
	void ImGuiInitialize();

	void LoadSampleDatas();
private:
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc_{};

	uint32_t descriptorSizeRTV_;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc_{};
public:
	void RegisterGlobalVariables();
	void ApplyGlobalVariables();

	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return device; };

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return commandList; };

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return commandQueue; };

	Microsoft::WRL::ComPtr<ID3D12Fence> GetFence() { return fence; };

	HANDLE GetFenceEvent() { return fenceEvent; };
	HWND GetHWND() { return winApp_->GetHWND(); };

	std::ofstream& GetLogStream() { return logStream; };

	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() { return swapChain; }

	D3D12_VIEWPORT GetViewport() { return viewport; };

	void SetViewport(D3D12_VIEWPORT setViewport) { viewport = setViewport; };

	void SetPipeline(BlendMode blendMode,ShaderType shaderType);

	void SetParticlePipeline(BlendMode blendMode);

	void DrawCommand(
		BlendMode blendMode,
		ShaderType shaderType,
		D3D12_VERTEX_BUFFER_VIEW* vertexBufferView,
		D3D12_INDEX_BUFFER_VIEW* indexBufferView,
		D3D_PRIMITIVE_TOPOLOGY topology,
		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource,
		Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource,
		D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU,
		uint32_t indexInstancedNum);
public:
	// ログを表示する.
	static void Log(const std::string& message);

	static void ExportLog(const std::string& message);

	// BufferResourceを作る関数.
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// DescriptorHeap関数(どうやってファイル分けするかね).
	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDiscriptors, bool shaderVisible);
	
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDiscriptors, bool shaderVisible);


	// Textureデータを読む(TextureManager的な奴に入れる).
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	// DirectX12のTextureResourceを作る(TextureManager的な奴に入れる).
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metaData);

	// にゅー！.
	static Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);
	//static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle();

	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);
	//static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle();

private:
	std::ofstream CreateLogFile();

	void WindowSizeUpdate();
private:
	/*=============================================================
	WindowsApi系
	=============================================================*/
	std::unique_ptr<WinApp> winApp_;

	/*=============================================================
	PSO
	=============================================================*/
	std::unique_ptr<Pipeline> pipeline_;


	uint32_t drawCount_;

	/*=============================================================
	ResourceLeakChecker
	=============================================================*/
	static D3DResourceLeakChecker resourceLeakChecker;

	MSG msg{};

	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

	std::ofstream logStream;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource[2] = { nullptr };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;

	uint64_t fenceValue = 0;

	HANDLE fenceEvent;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

	

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

	D3D12_RESOURCE_BARRIER barrier{};

	D3D12_VIEWPORT viewport{};

	D3D12_RECT scissorRect{};

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	//Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//
	//Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

};

