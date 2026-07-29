#include "FakeWindow.h"
void FakeWindow::Initialize() {
	transform_.Initialize();
	back_.Initialize(TextureManager::GetInstance()->GetTextureInfo("window_back"));
	mask_.Initialize(TextureManager::GetInstance()->GetTextureInfo("window_mask"));
	back_.SetBlendMode(BlendMode::kStencilNoneNormal);
	back_.SetTranslateZ(-200);
	mask_.SetBlendMode(BlendMode::kStencil);
	mask_.SetTranslateZ(-100);

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

void FakeWindow::SetType(WindowType type){
	type_ = type;
	switch (type){
	case kWindowTypeNormal:
		back_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("window_back"));
		mask_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("window_mask"));
		break;				
	case kWindowTypeSquareL:
		back_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("window_s_l_back"));
		mask_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("window_s_l_mask"));
		break;
	case kWindowTypeSquareS:
		back_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("window_s_s_back"));
		mask_.ChangeTexture(TextureManager::GetInstance()->GetTextureInfo("window_s_s_mask"));
		break;
	default:
		break;
	}
}
