#include "BaseEffect.h"
#include "HitEffect.h"

BaseEffect* BaseEffect::Create(Vector3 position, EffectType type) {
	BaseEffect* instance = nullptr;

	switch (type){
	case BaseEffect::EffectType::kHit:
		instance = new HitEffect();
		break;
	case BaseEffect::EffectType::kGuard:
		//instance = new GuardEffect();
		break;
	}

	assert(instance);

	instance->Initialize(position);

	return instance;
}
