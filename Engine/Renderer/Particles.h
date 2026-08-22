#pragma once
#include <Windows.h>
#include <cstdint>
#include "../../Managers/ModelManager.h"
#include "../../Managers/TextureManager.h"
#include "../../Environment.h"
#include "../../externals/DirectXTex/DirectXTex.h"
#include "../../externals/DirectXTex/d3dx12.h"
#include "../SystemFile/GameSystem.h"
#include "../Math/Matrix4x4.h"
#include "../Math/Field.h"
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

enum class BillboardType {
	kNone,
	kAllAxis, // 今はこれだけ対応.
	kOnlyX,
	kOnlyY,
	kOnlyZ,
};

class Particles{
public:
	enum class Move {
		kNormal,
		kFire,
		kSlash,
	};

	~Particles();
	void Initialize(const ModelInfo& info);

	void Initialize(const TextureInfo& info);

	void MakeNewParticle(const Vector3& position);

	void MakeNewParticle(const Transform& transform);

	void Update();

	void Draw();

	void SetBillboardType(BillboardType billboardType);
	void SetMoveType(Move moveType);

	BillboardType GetBillboardType() { return billboardType_; };
	
	void SetSize(const Vector3 size) { size_ = size; };

	void CheckCollision(const Field& field);

	void ClearParticle() { particleData_.clear(); };
private:
	void MoveNormal();
	void MoveFire();
private:
	Vector3 size_;

	Move moveType_;

	uint32_t modelMax_;
	static inline const uint32_t kNumMaxInstance = 100;

	Material* materialData_;
	bool isVisible_;
	std::list<ParticleData> particleData_;

	Matrix4x4 billboardMatrix_;


	ModelData modelData_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	VertexData* vertexData = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	Transform uvTransform_;

	BlendMode blendMode_;

	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;

	ParticleForGPU* instancingData_ = nullptr;


	uint32_t numInstance_;

	TextureInfo textures_;

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};

	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU;

	BillboardType billboardType_;
	//std::list<ParticleData> particleData_;
};

enum class EmitterShape {
	kBox,
	kSphere,
};

class Emitter {
public:
	void Initialize(const Transform& transform,uint32_t count,float frequency);

	void SetParticle(Particles* particles) { particles_ = particles; };

	void SetTransform(const Transform& transform) { transform_ = transform; };

	void SetCount(uint32_t count) { count_ = count; };

	void SetFrequency(float frequency) { frequency_ = frequency; };

	void CreateParticle();

	void Update();

	void DebugDraw();

	void SetShape(EmitterShape shape) { shape_ = shape; };
private:
	EmitterShape shape_;

	Particles* particles_;

	Transform transform_;
	uint32_t count_;
	float frequency_;
	float frequencyTime_;

	bool useTimer_;
};