#include "DirectionalLight.h"
#include "../SystemFile/GameSystem.h"
#include "../SystemFile/GlobalVariables.h"

//DirectionalLight* DirectionalLight::GetInstance() {
//	static DirectionalLight instance;
//	return &instance;
//};

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

void DirectionalLight::RegisterGlobalVariables() {
	//const std::string& groupName = "DirectionalLight";
	//
	//GlobalVariables::GetInstance()->CreateGroup(groupName);
	//
	//GlobalVariables::GetInstance()->AddValue(groupName,"Color", directionalLightData->color);
	//GlobalVariables::GetInstance()->AddValue(groupName,"Intensity", directionalLightData->intensity);
	//GlobalVariables::GetInstance()->AddValue(groupName,"Direction", directionalLightData->direction);
};

void DirectionalLight::ApplyGlobalVariables(){
	//const std::string& groupName = "DirectionalLight";
	//directionalLightData->color = GlobalVariables::GetInstance()->GetVector4Value(groupName, "Color");
	//directionalLightData->intensity = GlobalVariables::GetInstance()->GetFloatValue(groupName, "Intensity");
	//Vector3 direction = GlobalVariables::GetInstance()->GetVector3Value(groupName, "Direction");
	//directionalLightData->direction = direction.Normalize();
};