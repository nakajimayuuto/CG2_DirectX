#include "LightManager.h"
#include "../Engine/SystemFile/GameSystem.h"
LightManager* LightManager::GetInstance() {
	static LightManager instance;
	return &instance;
}

void LightData::Initialize(LightType newType) {
	color = {1.0f,1.0f,1.0f,1.0f}; // ライトの色.
	position = {0.0f,2.0f,0.0f}; // ライトの位置.
	intensity = 1.0f; // ライトの輝度.
	decay = 1.0f; // 減衰率.
	radius = 3.0f; // ライトの届く最大距離(これはPointLightのみ).
	direction = {0.0f,1.0f,0.0f}; // ライトの向き(これはSpotLightのみ).
	distance = 7.0f; // ライトの届く最大距離(これはSpotLightのみ).
	cosAngle = Radian(60.0f); // ライトの余弦(これはSpotLightのみ).
	cosFalloffStart = Radian(30.0f); // falloff(ライトの光が減衰し始める角度)の余弦(これはSpotLightのみ).
	type = newType; // PointLightかSpotLightか.

}

void LightManager::Initialize() {
	lightDatas_.clear();

	directionalLight_ = new DirectionalLight();
	pointLight_ = new PointLight();
	spotLight_ = new SpotLight();

	directionalLight_->Initialize();
	pointLight_->Initialize();
	spotLight_->Initialize(); 
	
	lightNumResource_ = GameSystem::CreateBufferResource(GameSystem::GetInstance()->GetDevice(), sizeof(LightNumData));
	// データを書き込む.
	// 書き込むためのアドレスを取得.
	lightNumResource_->Map(0, nullptr, reinterpret_cast<void**>(&lightNumData_));
	// 単位行列を書き込んでおく.
	lightNumData_->pointLightNum = 0;
	lightNumData_->spotLightNum = 0;
}

void LightManager::Update() {
	lightNumData_->pointLightNum = 0;
	lightNumData_->spotLightNum = 0;

	PointLightData pointLightData;
	SpotLightData spotLightData;

	for (std::pair<std::string, LightData*> lightData : lightDatas_) {
		switch (lightData.second->type) {
		case LightType::kPoint:
			pointLightData.position = lightData.second->position;
			pointLightData.radius = lightData.second->radius;
			pointLightData.intensity = lightData.second->intensity;
			pointLightData.decay = lightData.second->decay;
			pointLightData.color = lightData.second->color;
			pointLight_->SetPointLightData(lightNumData_->pointLightNum, pointLightData);

			lightNumData_->pointLightNum++;
			break;
		case LightType::kSpot:
			spotLightData.position = lightData.second->position;
			spotLightData.intensity = lightData.second->intensity;
			spotLightData.decay = lightData.second->decay;
			spotLightData.color = lightData.second->color;
			spotLightData.cosAngle = lightData.second->cosAngle;
			spotLightData.cosFalloffStart = lightData.second->cosFalloffStart;
			spotLightData.direction = lightData.second->direction;
			spotLightData.distance = lightData.second->distance;

			spotLight_->SetSpotLightData(lightNumData_->spotLightNum, spotLightData);

			lightNumData_->spotLightNum++;
			break;
		}

	}

	lightNum.pointLightNum = lightNumData_->pointLightNum;
	lightNum.spotLightNum = lightNumData_->spotLightNum;
}

LightData* LightManager::GetLightData(std::string name){
	auto it = lightDatas_.find(name);

	assert(it != lightDatas_.end(), std::format("name : {}と一致するlightDataが見つかりませんでした", name));

	return lightDatas_[name];
}

void LightManager::CreatePointLight(std::string name){
	if (lightNum.pointLightNum >= pointLight_->GetLightMax()) {
		assert(false,"pointLightの同時設置数の上限を超えました。");
	}

	if (lightDatas_.find(name) != lightDatas_.end()) {
		return;
	}

	lightDatas_[name] = new LightData();
	lightDatas_[name]->Initialize(LightType::kPoint);
}

void LightManager::CreateSpotLight(std::string name){
	if (lightNum.spotLightNum >= pointLight_->GetLightMax()) {
		assert(false, "pointLightの同時設置数の上限を超えました。");
	}

	if (lightDatas_.find(name) != lightDatas_.end()) {
		return;
	}

	lightDatas_[name] = new LightData();
	lightDatas_[name]->Initialize(LightType::kSpot);

}
