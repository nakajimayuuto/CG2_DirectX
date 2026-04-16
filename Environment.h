#pragma once
#include <cstdint>
#include <d3d12.h>

class WindowSize {
public:
public:
	uint32_t width;
	uint32_t height;
};

class Environment{
public:
	static Environment* GetInstance();

	LPCWSTR GetWindowTitle() { return kWindowTitle_; };
	WindowSize GetWindowSize() { return kWindowSize_; };

private:
	const LPCWSTR kWindowTitle_ = L"CG2WindowClass";

	const WindowSize kWindowSize_ = {1280,720};
};