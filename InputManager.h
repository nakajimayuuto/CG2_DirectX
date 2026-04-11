#pragma once

#define DIRECTINPUT_VERSION	0x0800 // DirectInputのバージョン指定.
#include <dinput.h>
#include <cstdint>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class InputManager {
public:
	static InputManager* GetInstance();

	void Initialize();

	void Update();

	bool PressKey(uint8_t inputKey) { return keys_[inputKey]; };

	bool TriggerKey(uint8_t inputKey) { return keys_[inputKey] && !preKeys_[inputKey]; };
	
	bool ReleaseKey(uint8_t inputKey) { return !keys_[inputKey] && preKeys_[inputKey]; };

	bool NoneKey(uint8_t inputKey) { return !keys_[inputKey] && !preKeys_[inputKey]; };

private:
	IDirectInput8* directInput_ = nullptr;

	IDirectInputDevice8* keyBoard = nullptr;

	BYTE keys_[256] = {};
	BYTE preKeys_[256] = {};
};

