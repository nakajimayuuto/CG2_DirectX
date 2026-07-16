#include "FakeWindow.h"
void FakeWindow::Initialize() {
	transform_.Initialize();
	back_.Initialize(TextureManager::GetInstance()->GetTextureInfo("window_back"));
	mask_.Initialize(TextureManager::GetInstance()->GetTextureInfo("window_mask"));
	transform_.translate.x = (back_.GetSize().x / 2.0f);
	transform_.translate.y = (back_.GetSize().y / 2.0f);
	back_.SetBlendMode(BlendMode::kStencilNoneNormal);
	mask_.SetBlendMode(BlendMode::kStencil);

	isActive_ = true;
}

void FakeWindow::Update() {

}

void FakeWindow::DrawBack() {
	back_.Draw(transform_);
}
void FakeWindow::DrawMask(){
	mask_.Draw(transform_);
}