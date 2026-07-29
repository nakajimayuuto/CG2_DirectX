#pragma once
#include "Satlib.h"

enum WindowType {
	kWindowTypeNormal,
	kWindowTypeSquareL,
	kWindowTypeSquareS,
	kWindowTypeCount,
};

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

	Vector2 GetWindowSize() { return back_.GetSize(); };

	void SetType(WindowType type);

	WindowType GetType() { return type_; };

	Vector2 GetWindowClientSize() { return {mask_.GetTextureInfo().width - 2.0f,mask_.GetTextureInfo().height - 32.0f}; };
private:
	Transform2D transform_;
	Renderer::Sprite back_;
	Renderer::Sprite mask_;

	WindowType type_;
	bool isActive_;
};

