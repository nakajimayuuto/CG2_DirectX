#pragma once
#include <map>
#include <string>
#include "../Engine/Renderer/Particles.h"
#include "../Engine/Math/Field.h"

class ParticleManager{
public:
	void Initialize();

	void Update();

	void Draw();

	void CreateNewParticles(const std::string& name ,const TextureInfo& info);
	void CreateNewParticles(const std::string& name, const ModelInfo& info);

	void CreateNewEmitter();

	void CreateNewField();
private:
	std::map<std::string, Particles> particles_;
	std::map<std::string, Emitter> emitters_;
};

