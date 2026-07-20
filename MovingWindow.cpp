#include "MovingWindow.h"
void MovingWindow::Initialize() {
	transform_.Initialize();

	fakeWindow_.Initialize();

}

void MovingWindow::Update() {
	fakeWindow_.Update();
	fakeWindow_.SetTransform(transform_);
}

void MovingWindow::DrawBack() {
	fakeWindow_.DrawBack();
}

void MovingWindow::DrawMask() {
	fakeWindow_.DrawMask();
}

void MovingWindow::OnCollision(Vector2 position){
	if (position.x <= -(fakeWindow_.GetWindowClientSize().x / 2.0f) + transform_.translate.x) {

	}
	if (position.x >= (fakeWindow_.GetWindowClientSize().x / 2.0f) + transform_.translate.x) {

	}
	if (position.y >= ((fakeWindow_.GetWindowClientSize().y / 2.0f) - 28.0f) + transform_.translate.y) {

	}
	if (position.x <= (fakeWindow_.GetWindowClientSize().x / 2.0f) - 28.0f + transform_.translate.y) {

	}
}