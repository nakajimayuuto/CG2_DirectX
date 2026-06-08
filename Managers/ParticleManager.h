#pragma once
#include <map>
#include <string>
#include "../Engine/Renderer/Particles.h"
#include "../Engine/Math/Field.h"

class ParticleManager{
public:
	static ParticleManager* GetInstance();

	void Initialize();

	void Update();

	void Draw();

	void CreateNewParticles(const std::string& name ,const TextureInfo& info);
	void CreateNewParticles(const std::string& name, const ModelInfo& info);

	void CreateNewEmitter(const std::string& name, const std::string& setParticleName, const Transform& transform, uint32_t count, float frequency);
	void CreateNewEmitter(const std::string& name, const std::string& setParticleName);

	void CreateNewField(const std::string& name, const AABB& aabb, const Vector3& acceleration);

	void SetBillboardType(BillboardType type);

	void SetBillboardType(const std::string& name, BillboardType type);

	void SetEmitterTransform(const std::string& name, const Transform& transform);

	void SetEmitterCount(const std::string& name, uint32_t count);

	void SetEmitterFrequency(const std::string& name, float frequency);

	void SetFieldArea(const std::string& name, const AABB& area);

	void SetFieldAcceleration(const std::string& name, const Vector3& acceleration);

	Particles* GetParticles(const std::string& name);
private:
	void CheckCollision();
private:
	std::map<std::string, Particles*> particles_;
	std::map<std::string, Emitter*> emitters_;
	std::map<std::string, Field*> fields_;
};

