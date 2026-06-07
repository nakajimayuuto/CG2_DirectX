#include "ParticleManager.h"

void ParticleManager::Initialize() {
	particles_.clear();
}

void ParticleManager::Update() {

	for (std::pair<std::string, Emitter> emitter :emitters_) {
		emitter.second.Update();
	}

	for (std::pair<std::string, Particles> particles : particles_) {
		particles.second.Update();
	}

}

void ParticleManager::Draw() {
	for (std::pair<std::string, Emitter> emitter : emitters_) {
		emitter.second.DebugDraw();
	}

	for (std::pair<std::string, Particles> particles : particles_) {
		particles.second.Draw();
	}
}

void ParticleManager::CreateNewParticles(const std::string& name, const TextureInfo& info){
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name].Initialize(info);
}

void ParticleManager::CreateNewParticles(const std::string& name, const ModelInfo& info){
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name].Initialize(info);
}
