#pragma once
#include "../Engine/Renderer/DirectionalLight.h"
#include "../Engine/Renderer/PointLight.h"
#include "../Engine/Renderer/SpotLight.h"
#include <map>

struct LightNumData {
	int32_t pointLightNum;
	int32_t spotLightNum;
};

enum class LightType{
	kPoint,
	kSpot,
};

class LightData {
public:
	void Initialize(LightType newType);
public:
	Vector4 color; // ライトの色.
	Vector3 position; // ライトの位置.
	float intensity; // ライトの輝度.
	float decay; // 減衰率.
	float radius; // ライトの届く最大距離(これはPointLightのみ).
	Vector3 direction; // ライトの向き(これはSpotLightのみ).
	float distance; // ライトの届く最大距離(これはSpotLightのみ).
	float cosAngle; // ライトの余弦(これはSpotLightのみ).
	float cosFalloffStart; // falloff(ライトの光が減衰し始める角度)の余弦(これはSpotLightのみ).
	LightType type; // PointLightかSpotLightか.
	bool isActive;
};

class LightManager{
public:
	static LightManager* GetInstance();
	
	void Initialize();

	DirectionalLightData* GetDirectionalLightData() { return directionalLight_->GetDirectionalLightData(); }

	void Update();
	
	LightData* GetLightData(std::string name);

	void CreatePointLight(std::string name);

	void CreateSpotLight(std::string name);

	void SetLightPos(std::string name,const Vector3 pos);

	void SetLightIsActive(std::string name,bool isActive);

	Microsoft::WRL::ComPtr<ID3D12Resource> GetLightNumResource() { return lightNumResource_; };

	Microsoft::WRL::ComPtr<ID3D12Resource> GetDirectionalLightResource() { return directionalLight_->GetDirectionalLightResource(); };

	D3D12_GPU_DESCRIPTOR_HANDLE GetPointLightSrvHandleGPU() { return pointLight_->GetSrvHandleGPU(); };
	D3D12_GPU_DESCRIPTOR_HANDLE GetSpotLightSrvHandleGPU() { return spotLight_->GetSrvHandleGPU(); };

	void ClearLight();

	void DeleteLight(std::string name);
private:
	std::map<std::string, LightData*> lightDatas_;

	DirectionalLight* directionalLight_;
	PointLight* pointLight_;
	SpotLight* spotLight_;

	Microsoft::WRL::ComPtr<ID3D12Resource> lightNumResource_ = nullptr;

	LightNumData* lightNumData_ = nullptr;
	LightNumData lightNum;
};

