#include "SpotLight.h"
#include "../SystemFile/GameSystem.h"
#include "../Math/Math.h"

SpotLight* SpotLight::GetInstance() {
	static SpotLight instance;
	return &instance;
}

void SpotLight::Initialize() {
	spotLightResource = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(SpotLightData));
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
};