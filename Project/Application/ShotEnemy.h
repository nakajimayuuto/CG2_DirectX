#pragma once
#include "IEnemy.h"
class ShotEnemy : public IEnemy{
public:
	void Initialize() override;

	void Update() override;

	void Draw() override;
};

