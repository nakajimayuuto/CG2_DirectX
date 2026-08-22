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


void ParticleManager::SpawnParticles(const std::string& name, const Transform transform){

	particles_[name]->MakeNewParticle(transform);

}

void ParticleManager::SpawnParticles(const std::string& name, const Vector3 vector3){

	particles_[name]->MakeNewParticle(vector3);
}

void ParticleManager::CreateNewParticles(const std::string& name, const TextureInfo& info){
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = new Particles();
	particles_[name]->Initialize(info);
}

void ParticleManager::CreateNewParticles(const std::string& name, const TextureInfo& info, BillboardType billType, Particles::Move moveType){
	if (particles_.find(name) != particles_.end()) {
		return;
	}

	particles_[name] = new Particles();
	particles_[name]->Initialize(info);
	particles_[name]->SetBillboardType(billType);
	particles_[name]->SetMoveType(moveType);
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

	//assert(it != particles_.end(), std::format("name : {}と一致するParticlesが見つかりませんでした", name));

	if (particles_[name]->GetBillboardType() == type) {
		return;
	}

	particles_[name]->SetBillboardType(type);
}

void ParticleManager::SetParticleSize(const std::string& name, const Vector3 size){
	auto it = particles_.find(name);

	particles_[name]->SetSize(size);
}

void ParticleManager::SetEmitterTransform(const std::string& name, const Transform& transform){
	auto it = emitters_.find(name);

#ifdef _DEBUG

	assert(it != emitters_.end(), std::format("name : {}と一致するEmitterが見つかりませんでした", name));

#endif // _DEBUG

	emitters_[name]->SetTransform(transform);
}

void ParticleManager::SetEmitterCount(const std::string& name, uint32_t count){
	auto it = emitters_.find(name);

#ifdef _DEBUG
	assert(it != emitters_.end(), std::format("name : {}と一致するEmitterが見つかりませんでした", name));
#endif //_DEBUG

	emitters_[name]->SetCount(count);
}

void ParticleManager::SetEmitterFrequency(const std::string& name, float frequency){
	auto it = emitters_.find(name);

#ifdef _DEBUG

	assert(it != emitters_.end(), std::format("name : {}と一致するEmitterが見つかりませんでした", name));

#endif // _DEBUG

	emitters_[name]->SetFrequency(frequency);
}

void ParticleManager::SetEmitterShape(const std::string& name, EmitterShape shape){
	auto it = emitters_.find(name);
#ifdef _DEBUG
	assert(it != emitters_.end(), std::format("name : {}と一致するEmitterが見つかりませんでした", name));
#endif // _DEBUG
	emitters_[name]->SetShape(shape);
}

void ParticleManager::SetFieldArea(const std::string& name, const AABB& area){
	auto it = fields_.find(name);
#ifdef _DEBUG
	assert(it != fields_.end(), std::format("name : {}と一致するFieldが見つかりませんでした", name));
#endif // _DEBUG
	fields_[name]->SetArea(area);

}

void ParticleManager::SetFieldAcceleration(const std::string& name, const Vector3& acceleration){
	auto it = fields_.find(name);
#ifdef _DEBUG
	assert(it != fields_.end(), std::format("name : {}と一致するFieldが見つかりませんでした", name));
#endif // _DEBUG
	fields_[name]->SetAcceleration(acceleration);
}

void ParticleManager::SetFieldIsActive(const std::string& name, bool isActive){
	auto it = fields_.find(name);
#ifdef _DEBUG
	assert(it != fields_.end(), std::format("name : {}と一致するFieldが見つかりませんでした", name));
#endif // _DEBUG
	fields_[name]->SetIsActive(isActive);
}

Particles* ParticleManager::GetParticles(const std::string& name){
	auto it = particles_.find(name);
#ifdef _DEBUG
	assert(it != particles_.end(), std::format("name : {}と一致するParticlesが見つかりませんでした",name ));
#endif // _DEBUG
	return it->second;
}

void ParticleManager::ClearParticles(){
	for (std::pair<std::string, Particles*> particles : particles_) {
		particles.second->ClearParticle();
	}
}
