#include "Fade.h"
#include <algorithm>
void Fade::Initialize() {
	sprite_.Initialize();
	sprite_.SetSize(Vector2(1280.0f, 720.0f));
	sprite_.SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
}

void Fade::Update() {
	switch (status_) {
	case Fade::Status::None:
		break;
	case Fade::Status::FadeIn:
		FadeInUpdate();
		break;
	case Fade::Status::FadeOut:
		FadeOutUpdate();
		break;
	default:
		break;
	}
}

void Fade::FadeInUpdate() {
	counter_ += DeltaTime::GetInstance()->GetDeltaTime();

	if (counter_ >= duration_) {
		counter_ = duration_;
	}

	sprite_.SetAlpha(std::clamp(1.0f - (counter_ / duration_), 0.0f, 1.0f));
}

void Fade::FadeOutUpdate() {
	counter_ += DeltaTime::GetInstance()->GetDeltaTime();

	if (counter_ >= duration_) {
		counter_ = duration_;
	}

	sprite_.SetAlpha(std::clamp(counter_ / duration_, 0.0f, 1.0f));
}

void Fade::Draw() {
	if (status_ == Status::None) {
		return;
	}

	sprite_.Draw(Transform::GetInitialValue());
}

void Fade::Start(Fade::Status status, float duration) {
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;

}

void Fade::Stop() { status_ = Fade::Status::None; }

bool Fade::isFinished() {
	switch (status_) {
	case Fade::Status::FadeIn:
	case Fade::Status::FadeOut:
		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}

	return true;
}
