#include "Fade.h"

void Fade::Initialize(){
	sprite_.Initialize();
	sprite_.SetSize(Environment::GetInstance()->GetWindowSize());
	sprite_.SetColor({0.0f,0.0f,0.0f,1.0f});

	isFinished_ = true;
}

void Fade::Update(){
	switch (status_){
	case Fade::Status::None:
		isFinished_ = true;
		break;
	case Fade::Status::FadeIn:
		counter_ += 1.0f / 60.0f; // こういうのは後々deltaTimeとかに置き換えましょうね。

		counter_ = std::min(counter_, duration_);

		isFinished_ = (counter_ >= duration_);

		sprite_.SetColor(Vector4(0.0f, 0.0f, 0.0f,1.0f - std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		break;
	case Fade::Status::FadeOut:
		counter_ += 1.0f / 60.0f; // こういうのは後々deltaTimeとかに置き換えましょうね。

		counter_ = std::min(counter_, duration_);

		isFinished_ = (counter_ >= duration_);

		sprite_.SetColor(Vector4(0.0f, 0.0f, 0.0f, std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		break;
	}
}

void Fade::Draw(){
	if (status_ == Status::None) {
		return;
	}

	sprite_.Draw({ {1.0f,1.0f},0.0f,{0.0f,0.0f} });
}

void Fade::Start(Status status, float duration){
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
	isFinished_ = false;
}

void Fade::Stop(){
	status_ = Status::None;
}
