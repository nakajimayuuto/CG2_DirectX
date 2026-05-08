#include "HitEffect.h"

HitEffect* HitEffect::Create(Vector3 position){
	HitEffect* instance = new HitEffect();
	assert(instance);

	instance->Initialize(position);

	return instance;
}

void HitEffect::Initialize(Vector3 position){
	model_.Initialize(ModelManager::GetInstance()->GetModelInfo("hit_effect_plane"));
	transformCircle_.Initialize();
	transformCircle_.translate = position;
}

void HitEffect::Update(){
}

void HitEffect::Draw(){
	model_.Draw(transformCircle_);
}
