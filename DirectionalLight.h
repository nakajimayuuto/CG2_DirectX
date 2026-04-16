#pragma once
#include "Vector4.h"
#include "Vector3.h"
#include <d3d12.h>

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

struct DirectionalLightData {
	Vector4 color; // ライトの色.
	Vector3 direction; // ライトの向き.
	float intensity; // ライトの輝度.
};

class DirectionalLight {
public:
	static DirectionalLight* GetInstance();

	void Initialize();

	Microsoft::WRL::ComPtr<ID3D12Resource> GetDirectionalLightResource() { return directionalLightResource; };

	DirectionalLightData* GetDirectionalLightData() { return directionalLightData; };
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = nullptr;

	DirectionalLightData* directionalLightData = nullptr;
};