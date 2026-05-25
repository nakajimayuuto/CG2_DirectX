#include "Environment.h"
#include "./Engine/SystemFile/GameSystem.h"

Environment* Environment::GetInstance() {
	static Environment gameSystem;
	return &gameSystem;
}

void Environment::Initialize() {
	monitorSize_.width = GetSystemMetrics(SM_CXSCREEN);
	monitorSize_.height = GetSystemMetrics(SM_CYSCREEN);

	aspect_ = static_cast<float>(kWindowSize_.width) / static_cast<float>(kWindowSize_.height);

	currentWindowMode_ = kWindowed;

	GetWindowRect(GameSystem::GetInstance()->GetHWND(),&windowRect);
}

void Environment::SetWindowMode(WindowMode mode) {
	if (mode == currentWindowMode_) {
		return;
	}

	switch (mode) {
	case kWindowed:
		//if (currentWindowMode_ == kExclusiveFullscreen) {
		//	GameSystem::GetInstance()->GetSwapChain().Get()->SetFullscreenState(FALSE, nullptr);
		//} else {
			SetWindowed();
		//}
		break;
	case kExclusiveFullscreen:
		//GameSystem::GetInstance()->GetSwapChain().Get()->SetFullscreenState(TRUE, nullptr);
		break;
	case kFullscreen:
		SetBorderlessFullscreen();
		break;
	default:
		break;
	}

	currentWindowMode_ = mode;
}

void Environment::SetBorderlessFullscreen(){
	GetWindowRect(GameSystem::GetInstance()->GetHWND(), &windowRect);
	SetWindowLong(GameSystem::GetInstance()->GetHWND(),GWL_STYLE,WS_POPUP);


	SetWindowPos(
		GameSystem::GetInstance()->GetHWND(),
		HWND_TOP,
		0,
		0,
		monitorSize_.width,
		monitorSize_.height,
		SWP_FRAMECHANGED | SWP_SHOWWINDOW
		);
}

void Environment::SetWindowed(){
	SetWindowLongW(GameSystem::GetInstance()->GetHWND(), GWL_STYLE, WS_OVERLAPPEDWINDOW);

	SetWindowPos(
		GameSystem::GetInstance()->GetHWND(),
		HWND_TOP,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		SWP_FRAMECHANGED | SWP_SHOWWINDOW
	);
}
