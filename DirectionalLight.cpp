#include "DirectionalLight.h"
#include "GameSystem.h"

DirectionalLight* DirectionalLight::GetInstance() {
	//static DirectionalLight instance;
	//return &instance;
	return nullptr;
};

void DirectionalLight::Initialize() {
	directionalLightResource = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(DirectionalLightData));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 単位行列を書き込んでおく.
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;
};