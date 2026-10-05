#pragma once
#include "Satlib.h"
#include <vector>
class IBossAttack {
public:
	~IBossAttack();
	virtual void Initialize(Transform* bossTransform,Transform*weaponTransform);

	virtual void Update(){};

	virtual void Draw(){};
protected:
protected:
	Transform* bossTransform_;
	std::vector<Transform*> weaponTransform_;
};