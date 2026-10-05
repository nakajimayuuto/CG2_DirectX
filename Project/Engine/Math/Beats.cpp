#include "Beats.h"

void Beats::Initialize() {
	deltaTimeBeats_ = 0.0f;
	beat1_ = 0.0f;
	beat1per16_ = 0.0f;
	beat1Timer_ = 0.0f;
	beat1per16Timer_ = 0.0f;
	beat1per16Judge_ = false;
	beat1Judge_ = false;
}

void Beats::Update() {
	beat1per16Timer_ += deltaTimeBeats_ * 4.0f;
	beat1Timer_ += deltaTimeBeats_ * 4.0f;

	if (beat1per16Timer_ >= 1.0f) {
		beat1per16Timer_ -= 1.0f;
		beat1per16_++;
		beat1per16Judge_ = true;
	} else {
		beat1per16Judge_ = false;
	}

	if (beat1Timer_ >= 16.0f) {
		beat1Timer_ -= 16.0f;
		beat1_++;
		beat1Judge_ = true;
	} else {
		beat1Judge_ = false;
	}
}