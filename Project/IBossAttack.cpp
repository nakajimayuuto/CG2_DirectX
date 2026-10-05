#include "IBossAttack.h"

IBossAttack::~IBossAttack(){
	delete bossTransform_;
	for (Transform* transform : weaponTransform_) {
		delete transform;
	}

	weaponTransform_.clear();
}

void IBossAttack::Initialize(Transform* bossTransform, Transform* weaponTransform){
	bossTransform_ = bossTransform;
	weaponTransform_.push_back(weaponTransform);
}
