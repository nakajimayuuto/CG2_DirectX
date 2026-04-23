#include "InputManager.h"
#include "GameSystem.h"

InputManager* InputManager::GetInstance() {
	static InputManager instance;
	return &instance;
}

void InputManager::Initialize() {
	// DirectInputの初期化.
	HRESULT result = DirectInput8Create(GameSystem::GetInstance()->GetWc().hInstance,DIRECTINPUT_VERSION,IID_IDirectInput8,
		(void**)&directInput_,nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイスの作成.
	result = directInput_->CreateDevice(GUID_SysKeyboard,&keyBoard,NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のリセット.
	result = keyBoard->SetDataFormat(&c_dfDIKeyboard); // 標準形式.
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット.
	result = keyBoard->SetCooperativeLevel(
		GameSystem::GetInstance()->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void InputManager::Update() {
	keyBoard->Acquire();
	memcpy(preKeys_, keys_, 256);
	keyBoard->GetDeviceState(sizeof(keys_),keys_);
}
