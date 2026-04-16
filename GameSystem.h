#pragma once
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")
#pragma comment(lib,"Dbghelp.lib")

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
#include "Vector4.h"
#include "Vertex.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Camera.h"
#include "Math.h"
#include "Environment.h"

#include "Material.h"
#include "DirectionalLight.h"
#include "Convert.h"

#include "ModelData.h"
#include <fstream>
#include <sstream>

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

#include "ModelManager.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI

class GameSystem {

	// GameSystemで使うやつ.
public:
	static GameSystem* GetInstance();
	
	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	void Initialize();

	bool ProcessMessage();

	bool BeginFrame();

	void DrawSetup();

	void EndFrame();

	void Finalize();

	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return device; };

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return commandList; };

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return commandQueue; };

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator() { return commandAllocator; };

	Microsoft::WRL::ComPtr<ID3D12Fence> GetFence() { return fence; };

	uint64_t GetFenceValue() { return fenceValue; };

	void FenceValueIncrement() { fenceValue++; };
	
	HANDLE GetFenceEvent() { return fenceEvent; };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSrvDescriptorHeap() { return srvDescriptorHeap; };
	uint32_t GetDescriptorSizeSRV() { return descriptorSizeSRV; };

	WNDCLASS GetWc() { return wc; };

	HWND GetHWND() { return hwnd; };

	std::ofstream& GetLogStream() { return logStream; };
private:

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

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	std::ofstream CreateLogFile();

public:
	// ログを表示する.
	static void Log(const std::string& message);

	// CompileShader関数(どうやってファイル分けするかね).
	static IDxcBlob* CompileShader(
		// CompilerするShaderファイルへのパス.
		const std::wstring& filePath,
		// Compilerに使用するProfile.
		const wchar_t* profile,
		// 初期化で生成したものを3つ.
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler);

	// BufferResourceを作る関数.
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

	// DescriptorHeap関数(どうやってファイル分けするかね).
	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDiscriptors, bool shaderVisible);



	// Textureデータを読む(TextureManager的な奴に入れる).
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	// DirectX12のTextureResourceを作る(TextureManager的な奴に入れる).
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metaData);

	// にゅー！.
	static Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

private:
	WNDCLASS wc{};

	MSG msg{};

	HRESULT hr;

	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

	HWND hwnd;

	std::ofstream logStream;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource[2] = { nullptr };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;

	uint64_t fenceValue = 0;

	HANDLE fenceEvent;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

	uint32_t descriptorSizeSRV;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

	D3D12_RESOURCE_BARRIER barrier{};

	D3D12_VIEWPORT viewport{};

	D3D12_RECT scissorRect{};

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};

