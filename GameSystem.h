#pragma once
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>
#include <dxcapi.h>
#include <vector>
#include <numbers>
#include "Vector4.h"
#include "Vertex.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Camera.h"
#include "Math.h"

#include "Material.h"
#include "DirectionalLight.h"
#include "Convert.h"

#include "ModelData.h"
#include <fstream>
#include <sstream>

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI

class GameSystem{
public:
	static GameSystem* GetInstance();
	
	void Initialize();

	void BeginFrame();

	void DrawSetup();

	void Endframe();

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

	// ログを表示する.
	void Log(const std::string& message);

	// CompileShader関数(どうやってファイル分けするかね).
	IDxcBlob* CompileShader(
		// CompilerするShaderファイルへのパス.
		const std::wstring& filePath,
		// Compilerに使用するProfile.
		const wchar_t* profile,
		// 初期化で生成したものを3つ.
		IDxcUtils* dxcUtils,
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler);

	// BufferResourceを作る関数.
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

	// DescriptorHeap関数(どうやってファイル分けするかね).
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDiscriptors, bool shaderVisible);



	// Textureデータを読む(TextureManager的な奴に入れる).
	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	// DirectX12のTextureResourceを作る(TextureManager的な奴に入れる).
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metaData);

	// にゅー！.
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, int32_t width, int32_t height);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDiscriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDiscriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index);

	// ModelManager的n(以下略.
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName);

	// ModelManager的な奴に入れる.
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& fileName);
};

