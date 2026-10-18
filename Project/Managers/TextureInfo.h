#pragma once

#include "../externals/DirectXTex/DirectXTex.h"
#include "../externals/DirectXTex/d3dx12.h"

struct TextureInfo {
	uint32_t number;

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = nullptr;

	uint32_t width;
	uint32_t height;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandlesCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandlesGPU;
};