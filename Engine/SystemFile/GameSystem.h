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


#include "../Math/Vector4.h"
#include "../Math/Vertex.h"
#include "../Math/Matrix4x4.h"
#include "../Math/Transform.h"
#include "../Math/Math.h"
#include "../../Environment.h"

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

enum class BlendMode {
	kNone, // ブレンドモード無し.
	kNormal, // 通常.
	kAdd, // 加算.
	kSubtract, // 減算.
	kMultily, // 乗算.
	kScreen, // スクリーン.

	kNormalCullNone, // 通常ブレンド。背面カリング無し.
	kLine, // 線の描画に使用.

	kParticleNone, // パーティクル用
	kParticleNormal, // パーティクル用の通常.
	kParticleCount,
	kCount, // ブレンドモードの最大数.
};

enum class ShaderType {
	kObject3d, // オブジェクト3D.
	kNoTexture, // テクスチャ無しモデル.
	kParticle, // パーティクル.
	kCount // 最大数.
};

struct ModelElement;

class GameSystem {
private:
	static inline const D3D12_FILTER kUsingFillter_ = D3D12_FILTER_MIN_MAG_MIP_POINT; // D3D12_FILTER_MIN_MAG_MIP_LINEAR.

	static inline const uint32_t kSrvDescriptorHeapNumMax = 128;
public:
	static GameSystem* GetInstance();

	static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);

	void Initialize();

	bool ProcessMessage();

	bool BeginFrame();

	void DrawSetup();

	void EndFrame();

	void Finalize();

	void RegisterGlobalVariables();
	void ApplyGlobalVariables();

	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return device; };

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return commandList; };

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() { return commandQueue; };

	//Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GetCommandAllocator() { return commandAllocator; };

	Microsoft::WRL::ComPtr<ID3D12Fence> GetFence() { return fence; };

	//uint64_t GetFenceValue() { return fenceValue; };

	//void FenceValueIncrement() { fenceValue++; };

	HANDLE GetFenceEvent() { return fenceEvent; };

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetSrvDescriptorHeap() { return srvDescriptorHeap; };
	uint32_t GetDescriptorSizeSRV() { return descriptorSizeSRV; };

	WNDCLASS GetWc() { return wc; };

	HWND GetHWND() { return hwnd; };

	std::ofstream& GetLogStream() { return logStream; };

	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() { return swapChain; }

	D3D12_VIEWPORT GetViewport() { return viewport; };

	void SetViewport(D3D12_VIEWPORT setViewport) { viewport = setViewport; };

	struct Pipeline {
		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
		Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	};

	// シェーダーの設定はここでやる.
	void CreatePipeline(BlendMode blendMode, ShaderType shaderType);

	void SetPipeline(BlendMode blendMode);

	void SetParticlePipeline(BlendMode blendMode);

	void DrawCommand(
		BlendMode blendMode,
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
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// DescriptorHeap関数(どうやってファイル分けするかね).
	static Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDiscriptors, bool shaderVisible);

	void SrvDescriptorHeapNumIncrement();

	uint32_t GetSrvDescriptorHeapNum()const { return srvDescriptorHeapNum_; };

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

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;

	uint32_t srvDescriptorHeapNum_;

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

	//Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	//
	//Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};

	Pipeline pipeline_[static_cast<uint32_t>(BlendMode::kCount) * static_cast<uint32_t>(ShaderType::kCount)];

	// Shaderをコンパイルする.
	// 【VertexShader】
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob3dObject;
	// 【PixelShader】
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob3dObject;
	// 【VertexShader】
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlobParticle;
	// 【PixelShader】
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlobParticle;
	// 【VertexShader】
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlobLine;
	// 【PixelShader】
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlobLine;
	// 【VertexShader】
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlobNoTexture;
	// 【PixelShader】
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlobNoTexture;
};

