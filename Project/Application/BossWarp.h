#pragma once
#include "IBossAttack.h"
class BossWarp : IBossAttack{
	void Initialize(Transform* bossTransform, Transform* weaponTransform) override;

	void Update() override;

	void Draw() override;
};

