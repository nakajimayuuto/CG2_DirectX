#pragma once
#include "Satlib.h"
class FakeWindow{
public:
	void Initialize();

	void Update();

	void DrawBack();
	void DrawMask();

	Transform2D GetTransform() {return transform_;};

	void SetTransform(Transform2D transform) { transform_ = transform; };

	Vector2 GetPosition() {return transform_.translate;};

	void SetPosition(Vector2 position) { transform_.translate = position; };

	bool GetIsActive() { return isActive_; }

	void SetIsActive(bool isActive) { isActive_ = isActive; }
private:
	Transform2D transform_;
	Renderer::Sprite back_;
	Renderer::Sprite mask_;

	bool isActive_;
};

