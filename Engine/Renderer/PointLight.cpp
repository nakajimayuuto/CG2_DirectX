#include "PointLight.h"
#include "../SystemFile/GameSystem.h"

PointLight* PointLight::GetInstance() {
	static PointLight instance;
	return &instance;
}

void PointLight::Initialize() {
	pointLightResource = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(DirectionalLightData));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	pointLightResource->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData));
	// 単位行列を書き込んでおく.
	pointLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	pointLightData->position = { 0.0f,2.0f,0.0f };
	pointLightData->intensity = 1.0f;
};