#pragma once
#include <d3d12.h>
#include <wrl.h>
struct RenderTextureData {
	// ゲーム画面を書き込むテクスチャ.
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	// RenderTexture用のRTV.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
	// RenderTexture用のSRV.
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU{};
};