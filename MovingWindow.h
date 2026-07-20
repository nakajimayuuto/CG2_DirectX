#pragma once
#include "Satlib.h"
#include "FakeWindow.h"

enum HitWindowDirection {
	kNoHit,
	kUpHit,
	kDownHit,
	kLeftHit,
	kLightHit,
};

class MovingWindow{
public:
	void Initialize();

	void Update();

	void DrawBack();
	void DrawMask();

	void OnCollision(Vector2 position);

private:
	FakeWindow fakeWindow_;

	Transform2D transform_;

	Vector2 acceleration_;
};

