#pragma once
#include <Windows.h>
#include <cstdint>
#include "../../Managers/ModelManager.h"
#include "../../Managers/TextureManager.h"
#include "../../Environment.h"
#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
#include "../SystemFile/GameSystem.h"
#include <list>

struct ParticleData {
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};

class Particles{
public:
	void Initialize(const ModelInfo& info, uint32_t numInstanced);

	ParticleData MakeNewParticle();

	void Update();

	void Draw(const Transform& transform);
private:
	uint32_t modelMax_;

	Material* materialData_;
	bool isVisible_;

	ModelData modelData_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	VertexData* vertexData = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Transform uvTransform_;

	BlendMode blendMode_;

	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	ParticleForGPU* instancingData_ = nullptr;

	static inline const uint32_t kNumMaxInstance = 10;

	uint32_t numInstance_;

	TextureInfo textures_;

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU;

	//std::list<ParticleData> particleData_;
	ParticleData particleData_[kNumMaxInstance];
};