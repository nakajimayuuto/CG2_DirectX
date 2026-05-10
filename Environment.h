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

	void GameFinished() { isGameFinished_ = true; };

	bool GetIsGameFinished() { return isGameFinished_; }

	LPCWSTR GetWindowTitle() { return kWindowTitle_; };
	WindowSize GetWindowSize() { return kWindowSize_; };

private:
	const LPCWSTR kWindowTitle_ = L"AL3_03_3Dレールアクション";

	const WindowSize kWindowSize_ = {1280,720};

	bool isGameFinished_ = false;
};