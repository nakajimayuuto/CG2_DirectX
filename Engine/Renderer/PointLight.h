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
};

class PointLight {
public:
	static PointLight* GetInstance();

	void Initialize();

	Microsoft::WRL::ComPtr<ID3D12Resource> GetPointLightResource() { return pointLightResource; };

	PointLightData* GetDirectionalLightData() { return pointLightData; };

	void RegisterGlobalVariables();
	void ApplyGlobalVariables();
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> pointLightResource = nullptr;

	PointLightData* pointLightData = nullptr;
};

