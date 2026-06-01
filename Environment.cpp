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

	aspectMode_ = kAspectFrameFixed;

	GetWindowRect(GameSystem::GetInstance()->GetHWND(), &windowRect);
	currentStyle = GetWindowLongPtr(GameSystem::GetInstance()->GetHWND(), GWL_STYLE);
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

void Environment::SetAspectMode(AspectMode aspectMode) {
	if (aspectMode_ == aspectMode) {
		return;
	}

	RECT currentRect = {0,0,0,0,};
	GetWindowRect(GameSystem::GetInstance()->GetHWND(), &currentRect);

	switch (aspectMode) {
	case kAspectNone:
	case kAspectWindowFixed:
		Camera::GetInstance()->SetWindowSize(kWindowSize_.width, kWindowSize_.height);
		currentStyle = WS_OVERLAPPEDWINDOW;

		SetWindowLongW(GameSystem::GetInstance()->GetHWND(), GWL_STYLE, currentStyle);
		SetWindowPos(
			GameSystem::GetInstance()->GetHWND(),
			HWND_TOP,
			currentRect.left,
			currentRect.top,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW
		);
		break;
	case kAspectNoChange:
		Camera::GetInstance()->SetWindowSize(kWindowSize_.width, kWindowSize_.height);

		currentStyle = 
			WS_OVERLAPPED | 
			WS_CAPTION | 
			WS_MINIMIZEBOX |
			WS_SYSMENU;

		SetWindowLongW(GameSystem::GetInstance()->GetHWND(), GWL_STYLE, currentStyle);
		SetWindowPos(
			GameSystem::GetInstance()->GetHWND(),
			HWND_TOP,
			currentRect.left,
			currentRect.top,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW
		);
		break;
	case kAspectFrameFixed:
	case kAspectChangeEverytime:
		currentStyle = WS_OVERLAPPEDWINDOW;

		SetWindowLongW(GameSystem::GetInstance()->GetHWND(), GWL_STYLE, currentStyle);
		SetWindowPos(
			GameSystem::GetInstance()->GetHWND(),
			HWND_TOP,
			currentRect.left,
			currentRect.top,
			currentRect.right - currentRect.left,
			currentRect.bottom - currentRect.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW
		);
		break;
	}

	aspectMode_ = aspectMode;
}

void Environment::SetBorderlessFullscreen() {
	GetWindowRect(GameSystem::GetInstance()->GetHWND(), &windowRect);
	SetWindowLong(GameSystem::GetInstance()->GetHWND(), GWL_STYLE, WS_POPUP);


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

void Environment::SetWindowed() {
	SetWindowLongW(GameSystem::GetInstance()->GetHWND(), GWL_STYLE, currentStyle);

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
