#pragma once
#include "../Math/Vector4.h"
#include "../Math/Vector3.h"
#include <d3d12.h>

#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"

struct SpotLightData {
	Vector4 color; // ライトの色.
	Vector3 position; // ライトの位置.
	float intensity; // ライトの輝度.
	Vector3 direction; // ライトの向き.
	float distance; // ライトの届く最大距離.
	float decay; // 減衰率.
	float cosAngle; // ライトの余弦.
	float cosFalloffStart; // falloff(ライトの光が減衰し始める角度)の余弦.
	float padding[2];
};

class SpotLight {
public:
	static SpotLight* GetInstance();

	void Initialize();

	Microsoft::WRL::ComPtr<ID3D12Resource> GetSpotLightResource() { return spotLightResource; };

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU() { return spotLightSrvHandleGPU; };

	SpotLightData* GetSpotLightData() { return spotLightData; };

	void RegisterGlobalVariables();
	void ApplyGlobalVariables();
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> spotLightResource = nullptr;

	SpotLightData* spotLightData = nullptr;

	D3D12_SHADER_RESOURCE_VIEW_DESC spotLightSrvDesc{};

	D3D12_CPU_DESCRIPTOR_HANDLE spotLightSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE spotLightSrvHandleGPU;
};

