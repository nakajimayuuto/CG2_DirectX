#include "SpotLight.h"
#include "../SystemFile/GameSystem.h"
#include "../Math/MyMath.h"

//SpotLight* SpotLight::GetInstance() {
//	static SpotLight instance;
//	return &instance;
//}

void SpotLight::Initialize() {
	spotLightResource = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(SpotLightData) * kLightMax);
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	spotLightResource->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData));
	// 単位行列を書き込んでおく.
	spotLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	spotLightData->position = { 2.0f,0.0f,0.0f };
	spotLightData->distance = 7.0f;
	spotLightData->direction = { -1.0f,0.0f,0.0f };
	spotLightData->intensity = 4.0f;
	spotLightData->decay = 2.0f;
	spotLightData->cosAngle = std::cos(Radian(60.0f));
	spotLightData->cosFalloffStart = std::cos(Radian(30.0f));

	spotLightSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	spotLightSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	spotLightSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	spotLightSrvDesc.Buffer.FirstElement = 0;
	spotLightSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	spotLightSrvDesc.Buffer.NumElements = kLightMax;
	spotLightSrvDesc.Buffer.StructureByteStride = sizeof(SpotLightData);

	spotLightSrvHandleCPU = GameSystem::GetInstance()->GetCPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), GameSystem::GetInstance()->GetSrvDescriptorHeapNum());
	spotLightSrvHandleGPU = GameSystem::GetInstance()->GetGPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), GameSystem::GetInstance()->GetSrvDescriptorHeapNum());

	GameSystem::GetInstance()->GetDevice()->CreateShaderResourceView(spotLightResource.Get(), &spotLightSrvDesc, spotLightSrvHandleCPU);

	GameSystem::GetInstance()->SrvDescriptorHeapNumIncrement();
};