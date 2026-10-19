#pragma once
#include "Satlib.h"
class Fade {
public:
	
	enum class Status {
		None,
		FadeIn,
		FadeOut,
	};
private:
	Sprite sprite_;

	Fade::Status status_ = Fade::Status::None;

	float duration_ = 0.0f;

	float counter_ = 0.0f;

public:

	void Initialize();

	void Update();

	void FadeInUpdate();

	void FadeOutUpdate();

	void Draw();

	void Start(Fade::Status status, float duration);

	void Stop();

	bool isFinished();

	void SetColor(Vector3 rgb) { sprite_.SetColor({rgb.x,rgb.y,rgb.z,sprite_.GetAlpha()}); };
};
