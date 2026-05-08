#pragma once
#include "Satlib.h"
#include "assert.h"

class HitEffect{
public:
	static HitEffect* Create(Vector3 position);

	void Initialize(Vector3 position);

	void Update();

	void Draw();
private:
	Renderer::Model model_;

	Transform transformCircle_;
};

