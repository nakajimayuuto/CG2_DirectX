#pragma once
#include <string>
#include <vector>


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

	// name のテクスチャが登録されているか.
	bool HasTexture(const std::string& name) const { return textures_.find(name) != textures_.end(); };

	// 登録されているテクスチャの名前の一覧(名前順).
	std::vector<std::string> GetTextureNames() const {
		std::vector<std::string> names;
		for (const auto& [name, info] : textures_) {
			names.push_back(name);
		}
		return names;
	};
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

