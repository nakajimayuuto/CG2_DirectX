#pragma once
#pragma comment(lib,"d3d12.lib")

#include <map>
#include <string>
#include "ModelData.h"
#include <fstream>
#include <sstream>


#include <d3d12.h>

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

struct ModelInfo {
	ModelData modelData;

	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandlesGPU;
};

class ModelManager{
public:
	static ModelManager* GetInstance();

	void RegisterObj(const std::string& name, const std::string& directoryPath, const std::string& fileName);
	
	ModelData GetModelData(const std::string& name);

	ModelInfo GetModelInfo(const std::string& name);
private:
	// ModelManager的n(以下	略.
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName);

	// ModelManager的な奴に入れる.
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& fileName);

	// Textureデータを読む(TextureManager的な奴に入れる).
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);

	// DirectX12のTextureResourceを作る(TextureManager的な奴に入れる).
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metaData);

	// にゅー！.
	static Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages, Microsoft::WRL::ComPtr<ID3D12Device> device,
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList);
private:
	std::map<std::string, ModelInfo> models_;
};

