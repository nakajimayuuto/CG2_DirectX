#pragma once
#include <string>


#include <map>
#include <d3d12.h>

#include "../externals/DirectXTex/DirectXTex.h"
#include "../externals/DirectXTex/d3dx12.h"
#include "TextureInfo.h"

class TextureManager{
public:
	static TextureManager* GetInstance();

	TextureInfo RegisterTexture(const std::string& name, const std::string& filePath);
	TextureInfo RegisterTexture(const std::string& name, const std::string& filePath,bool useMipMap);

	//TextureInfo RegisterParticleTexture(const std::string& name, const std::string& filePath);

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
	std::map<std::string, TextureInfo> particleTextures_;
	uint32_t textureNumber_ = 1;
};

