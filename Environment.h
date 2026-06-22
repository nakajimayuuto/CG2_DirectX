#pragma once
#include <cstdint>
#include <d3d12.h>

class WindowSize {
public:
public:
	uint32_t width;
	uint32_t height;
};

enum WindowMode {
	kWindowed, // ウィンドウの状態.
	kExclusiveFullscreen, //排他的フルスクリーン。せっかくだし作ろうとしたけどうまくいかなかった.
	kFullscreen, // フルスクリーン.
};

enum AspectMode {
	kAspectNone, // アスペクト比の調節無し.
	kAspectNoChange, // 変更できなくする.
	kAspectWindowFixed, // アスペクト比をウィンドウを変化させて調節する.
	kAspectFrameFixed,  // アスペクト比をフレームを作って調節する.
	kAspectChangeEverytime, // kAspectFrameFixedのフレーム無し版.
	kAspectCountMax // アスペクトモードの要素数.
};

class Environment{
public:
	static Environment* GetInstance();

	void Initialize();

	void GameFinished() { isGameFinished_ = true; };

	bool GetIsGameFinished() { return isGameFinished_; }

	LPCWSTR GetWindowTitle() { return kWindowTitle_; };
	WindowSize GetWindowSize() { return kWindowSize_; };

	void SetWindowMode(WindowMode mode);

	WindowMode GetWindowMode() { return currentWindowMode_; };

	void SetAspectMode(AspectMode aspectMode);

	AspectMode GetAspectMode() {
		return aspectMode_; 
	};

	float GetAspect() { return aspect_; }
private:
	void SetBorderlessFullscreen();

	void SetWindowed();
private:
	const LPCWSTR kWindowTitle_ = L"SaturnCGEngine";

	const WindowSize kWindowSize_ = { 1280,720 };//{1280,720};

	float aspect_;

	bool isGameFinished_ = false;

	WindowSize monitorSize_;

	WindowMode currentWindowMode_;

	AspectMode aspectMode_;

	LONG_PTR currentStyle;

	RECT windowRect{};
};