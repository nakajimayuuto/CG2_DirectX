#include "ParticleManager.h"

ParticleManager* ParticleManager::GetInstance(){
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Initialize() {
	particles_.clear();
	emitters_.clear();
	fields_.clear();
}

void ParticleManager::Update() {

	for (std::pair<std::string, Emitter*> emitter :emitters_) {
		emitter.second->Update();
	}

	CheckCollision();

	for (std::pair<std::string, Particles*> particles : particles_) {
		particles.second->Update();
	}

}

void ParticleManager::Draw() {
	for (std::pair<std::string, Emitter*> emitter : emitters_) {
		emitter.second->DebugDraw();
	}

	for (std::pair<std::string, Field*> field : fields_) {
		field.second->DebugDraw();
	}

	for (std::pair<std::string, Particles*> particles : particles_) {
		particles.second->Draw();
	}
}

void ParticleManager::CheckCollision() {
	for (std::pair<std::string, Field*> field : fields_) {
		for (std::pair<std::string, Particles*> particles : particles_) {
			particles.second->CheckCollision(*field.second);
		}
	}
}


void ParticleManager::CreateNewParticles(const std::string& name, const TextureInfo& info){
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = new Particles();
	particles_[name]->Initialize(info);
}

void ParticleManager::CreateNewParticles(const std::string& name, const ModelInfo& info){
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = new Particles();
	particles_[name]->Initialize(info);
}

void ParticleManager::CreateNewEmitter(const std::string& name, const std::string& setParticleName, const Transform& transform, uint32_t count, float frequency){
	if (emitters_.find(name) != emitters_.end()) {
		return;
	}

	emitters_[name] = new Emitter();
	emitters_[name]->Initialize(transform,count,frequency);
	emitters_[name]->SetParticle(GetParticles(setParticleName));
}

void ParticleManager::CreateNewEmitter(const std::string& name, const std::string& setParticleName){
	if (emitters_.find(name) != emitters_.end()) {
		return;
	}

	emitters_[name] = new Emitter();
	emitters_[name]->Initialize(Transform::GetInitialValue(), 3, 0.3f);
	emitters_[name]->SetParticle(GetParticles(setParticleName));
}

void ParticleManager::CreateNewField(const std::string& name, const AABB& aabb, const Vector3& acceleration){
	if (fields_.find(name) != fields_.end()) {
		return;
	}

	fields_[name] = new Field();
	fields_[name]->Initialize(acceleration,aabb);
}

void ParticleManager::SetBillboardType(BillboardType type){
	for (std::pair<std::string, Particles*> particles : particles_) {
		if(particles.second->GetBillboardType() == type){
			continue;
		}

		particles.second->SetBillboardType(type);
	}
}

void ParticleManager::SetBillboardType(const std::string& name, BillboardType type){
	auto it = particles_.find(name);

	assert(it != particles_.end(), std::format("name : {}と一致するParticlesが見つかりませんでした", name));

	if (particles_[name]->GetBillboardType() == type) {
		return;
	}

	particles_[name]->SetBillboardType(type);
}

void ParticleManager::SetEmitterTransform(const std::string& name, const Transform& transform){
	auto it = emitters_.find(name);

	assert(it != emitters_.end(), std::format("name : {}と一致するEmittersが見つかりませんでした", name));

	emitters_[name]->SetTransform(transform);
}

void ParticleManager::SetEmitterCount(const std::string& name, uint32_t count){
	auto it = emitters_.find(name);

	assert(it != emitters_.end(), std::format("name : {}と一致するEmittersが見つかりませんでした", name));

	emitters_[name]->SetCount(count);
}

void ParticleManager::SetEmitterFrequency(const std::string& name, float frequency){
	auto it = emitters_.find(name);

	assert(it != emitters_.end(), std::format("name : {}と一致するEmittersが見つかりませんでした", name));

	emitters_[name]->SetFrequency(frequency);
}

void ParticleManager::SetFieldArea(const std::string& name, const AABB& area){
	auto it = fields_.find(name);

	assert(it != fields_.end(), std::format("name : {}と一致するFieldsが見つかりませんでした", name));

	fields_[name]->SetArea(area);

}

void ParticleManager::SetFieldAcceleration(const std::string& name, const Vector3& acceleration){
	auto it = fields_.find(name);

	assert(it != fields_.end(), std::format("name : {}と一致するFieldsが見つかりませんでした", name));

	fields_[name]->SetAcceleration(acceleration);
}

Particles* ParticleManager::GetParticles(const std::string& name){
	auto it = particles_.find(name);

	assert(it != particles_.end(), std::format("name : {}と一致するParticlesが見つかりませんでした",name ));

	return it->second;
}
