#include "PointLight.h"
#include "../SystemFile/GameSystem.h"

PointLight* PointLight::GetInstance() {
	static PointLight instance;
	return &instance;
}

void PointLight::Initialize() {
	pointLightResource = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(PointLightData));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	// 単位行列を書き込んでおく.
	pointLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	pointLightData->position = { 0.0f,2.0f,0.0f };
	pointLightData->intensity = 1.0f;
	pointLightData->radius = 3.0f;
	pointLightData->decay = 1.0f;



	pointLightSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	pointLightSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	pointLightSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	pointLightSrvDesc.Buffer.FirstElement = 0;
	pointLightSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	pointLightSrvDesc.Buffer.NumElements = 1;
	pointLightSrvDesc.Buffer.StructureByteStride = sizeof(PointLightData);

	pointLightSrvHandleCPU = GameSystem::GetInstance()->GetCPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), GameSystem::GetInstance()->GetSrvDescriptorHeapNum());
	pointLightSrvHandleGPU = GameSystem::GetInstance()->GetGPUDescriptorHandle(GameSystem::GetInstance()->GetSrvDescriptorHeap(), GameSystem::GetInstance()->GetDescriptorSizeSRV(), GameSystem::GetInstance()->GetSrvDescriptorHeapNum());

	GameSystem::GetInstance()->GetDevice()->CreateShaderResourceView(pointLightResource.Get(), &pointLightSrvDesc, pointLightSrvHandleCPU);

	GameSystem::GetInstance()->SrvDescriptorHeapNumIncrement();
};