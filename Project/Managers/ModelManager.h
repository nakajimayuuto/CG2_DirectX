#pragma once

#include <map>
#include <string>
#include "../Engine/Renderer/ModelData.h"
#include <fstream>
#include <sstream>


#include <d3d12.h>

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

#include "../externals/DirectXTex/DirectXTex.h"
#include "../externals/DirectXTex/d3dx12.h"

struct ModelInfo {
	std::vector<ModelData> modelData;

	float radius;

	uint32_t index;
};

class ModelManager{
public:
	static ModelManager* GetInstance();

	void RegisterObj(const std::string& name, const std::string& directoryPath, const std::string& fileName);
	void RegisterObj(const std::string& name, const std::string& directoryPath, const std::string& fileName,bool useMipMap);
	
	ModelData GetModelData(const std::string& name);

	ModelInfo GetModelInfo(const std::string& name);
private:
	// ModelManager的n(以下	略.
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& fileName,const std::string& usemtl);

	// ModelManager的な奴に入れる.
	std::vector<ModelData> LoadObjFile(const std::string& directoryPath, const std::string& fileName);

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

