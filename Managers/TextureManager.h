#pragma once
#include <string>


#include <map>
#include <d3d12.h>

#include "../externals/DirectXTex/DirectXTex.h"
#include "../externals/DirectXTex/d3dx12.h"

struct TextureInfo {
	uint32_t number;

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = nullptr;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandlesCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandlesGPU;
};

class TextureManager{
public:
	static TextureManager* GetInstance();

	TextureInfo RegisterTexture(const std::string& name, const std::string& filePath);

	TextureInfo GetTextureInfo(const std::string& name);
private:
	// Textureデータを読む(TextureManager的な奴に入れる).
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	// DirectX12のTextureResourceを作る(TextureManager的な奴に入れる).
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metaData);

	// にゅー！.
	static Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
private:
	std::map<std::string, TextureInfo> textures_;
	uint32_t textureNumber_ = 1;
};

