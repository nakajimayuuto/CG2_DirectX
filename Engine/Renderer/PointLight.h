#pragma once
#include "../Math/Vector4.h"
#include "../Math/Vector3.h"
#include <d3d12.h>

#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"

struct PointLightData {
	Vector4 color; // ライトの色.
	Vector3 position; // ライトの向き.
	float intensity; // ライトの輝度.
	float radius; // ライトの届く最大距離.
	float decay; // 減衰率.
	float padding[0];
};

class PointLight {
public:
	//static PointLight* GetInstance();

	void Initialize();

	Microsoft::WRL::ComPtr<ID3D12Resource> GetPointLightResource() { return pointLightResource; };

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU() { return pointLightSrvHandleGPU; };

	PointLightData* GetPointLightData() { return pointLightData; };

	void SetPointLightData(uint32_t index, const PointLightData& data) { pointLightData[index] = data; };

	uint32_t GetLightMax()const { return kLightMax; };

	void RegisterGlobalVariables();
	void ApplyGlobalVariables();
private:
	static inline const uint32_t kLightMax = 64;
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource = nullptr;

	PointLightData* pointLightData = nullptr;

	D3D12_SHADER_RESOURCE_VIEW_DESC pointLightSrvDesc{};

	D3D12_CPU_DESCRIPTOR_HANDLE pointLightSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE pointLightSrvHandleGPU;
};

