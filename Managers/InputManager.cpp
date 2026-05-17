#include "InputManager.h"
#include "../Engine/SystemFile/GameSystem.h"
#include <algorithm>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment (lib, "xinput.lib")

InputManager* InputManager::GetInstance() {
	static InputManager instance;
	return &instance;
}

void InputKeyBoard::Initialize(IDirectInput8* directInput) {

	// キーボードデバイスの作成.
	HRESULT result = directInput->CreateDevice(GUID_SysKeyboard, &keyBoard_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のリセット.
	result = keyBoard_->SetDataFormat(&c_dfDIKeyboard); // 標準形式.
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット.
	result = keyBoard_->SetCooperativeLevel(
		GameSystem::GetInstance()->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void InputKeyBoard::Update() {
	keyBoard_->Acquire();
	memcpy(preKeys_, keys_, 256);
	keyBoard_->GetDeviceState(sizeof(keys_), keys_);
}

bool InputKeyBoard::IsOperationDevice(){
	for (size_t i = 0; i < 256; i++) {
		if (keys_[i]) {
			return true;
		}
	}

	return false;
}

void InputGamePad::Initialize() {
	ZeroMemory(&state_, sizeof(XINPUT_STATE));
	ZeroMemory(&preState_, sizeof(XINPUT_STATE));
}

void InputGamePad::Update() {

	preState_ = state_;

	DWORD result = XInputGetState(padNo_, &state_);

	isConnected_ = (result == ERROR_SUCCESS);

	if (!isConnected_) {
		ZeroMemory(&state_, sizeof(XINPUT_STATE));
	}
}

bool InputGamePad::IsConnected() const {
	return isConnected_;
}

Vector2 InputGamePad::GetLeftStickDirection() const {
	Vector2 result;
	result.x = static_cast<float>(state_.Gamepad.sThumbLX);
	result.y = static_cast<float>(state_.Gamepad.sThumbLY);

	if (GetLeftStickInclination() < kLeftStickDeadZone) {
		return { 0.0f,0.0f };
	}

	if (result.x == 0.0f && result.y == 0.0f) {
		return { 0.0f,0.0f };
	}
	return result.Normalize();
}

Vector2 InputGamePad::GetRightStickDirection() const {
	Vector2 result;
	result.x = static_cast<float>(state_.Gamepad.sThumbRX);
	result.y = static_cast<float>(state_.Gamepad.sThumbRY);

	if (GetRightStickInclination() < kRightStickDeadZone) {
		return { 0.0f,0.0f };
	}

	if (result.x == 0.0f && result.y == 0.0f) {
		return { 0.0f,0.0f };
	}
	return result.Normalize();
}

Vector2 InputGamePad::GetPreLeftStickDirection() const {
	Vector2 result;
	result.x = static_cast<float>(preState_.Gamepad.sThumbLX);
	result.y = static_cast<float>(preState_.Gamepad.sThumbLY);

	if (GetPreLeftStickInclination() < kLeftStickDeadZone) {
		return { 0.0f,0.0f };
	}

	if (result.x == 0.0f && result.y == 0.0f) {
		return { 0.0f,0.0f };
	}
	return result.Normalize();
}

Vector2 InputGamePad::GetPreRightStickDirection() const {
	Vector2 result;
	result.x = static_cast<float>(preState_.Gamepad.sThumbRX);
	result.y = static_cast<float>(preState_.Gamepad.sThumbRY);

	if (GetPreRightStickInclination() < kRightStickDeadZone) {
		return { 0.0f,0.0f };
	}

	if (result.x == 0.0f && result.y == 0.0f) {
		return {0.0f,0.0f};
	}
	return result.Normalize();
}

float InputGamePad::GetLeftStickInclination() const{
	Vector2 result;
	result.x = static_cast<float>(state_.Gamepad.sThumbLX) / 32768.0f;
	result.y = static_cast<float>(state_.Gamepad.sThumbLY) / 32768.0f;
	return result.Length();
}

float InputGamePad::GetRightStickInclination() const {
	Vector2 result;
	result.x = static_cast<float>(state_.Gamepad.sThumbRX) / 32768.0f;
	result.y = static_cast<float>(state_.Gamepad.sThumbRY) / 32768.0f;
	return result.Length();
}

float InputGamePad::GetPreLeftStickInclination() const {
	Vector2 result;
	result.x = static_cast<float>(preState_.Gamepad.sThumbLX) / 32768.0f;
	result.y = static_cast<float>(preState_.Gamepad.sThumbLY) / 32768.0f;
	return result.Length();
}

float InputGamePad::GetPreRightStickInclination() const {
	Vector2 result;
	result.x = static_cast<float>(preState_.Gamepad.sThumbRX) / 32768.0f;
	result.y = static_cast<float>(preState_.Gamepad.sThumbRY) / 32768.0f;
	return result.Length();
}

bool InputGamePad::IsOperationDevice(){
	for (size_t i = 0; i < INPUT_MAX; i++) {
		if (GetButtonPress(static_cast<PadButtoms>(i))) {
			return true;
		}
	}

	return false;
}

BYTE InputGamePad::GetButtonPress(PadButtoms button) const {
	if (button == INPUT_R2) {
		return state_.Gamepad.bRightTrigger >= kSinkingState_;
	} else if (button == INPUT_L2) {
		return state_.Gamepad.bLeftTrigger >= kSinkingState_;
	}

	if (GetLeftStickInclination() > kLeftStickDeadZone) {
		if (button == INPUT_LSTICK_UP) {
			if (GetLeftStickDirection().y > 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_DOWN) {
			if (GetLeftStickDirection().y < 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_LEFT) {
			if (GetLeftStickDirection().x < 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_RIGHT) {
			if (GetLeftStickDirection().x > 0.0f) {
				return true;
			}
			return false;
		}
	}

	if (GetRightStickInclination() > kLeftStickDeadZone) {
		if (button == INPUT_LSTICK_UP) {
			if (GetRightStickDirection().y > 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_DOWN) {
			if (GetRightStickDirection().y < 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_LEFT) {
			if (GetRightStickDirection().x < 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_RIGHT) {
			if (GetRightStickDirection().x > 0.0f) {
				return true;
			}
			return false;
		}
	}

	switch (button){
	case INPUT_A:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0) {
			return true;
		}
		break;
	case INPUT_B:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0) {
			return true;
		}
		break;
	case INPUT_X:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0) {
			return true;
		}
		break;
	case INPUT_Y:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0) {
			return true;
		}
		break;
	case INPUT_UP:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0) {
			return true;
		}
		break;
	case INPUT_DOWN:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0) {
			return true;
		}
		break;
	case INPUT_LEFT:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0) {
			return true;
		}
		break;
	case INPUT_RIGHT:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0) {
			return true;
		}
		break;
	case INPUT_L1:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0) {
			return true;
		}
		break;
	case INPUT_R1:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0) {
			return true;
		}
		break;
	case INPUT_L3:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0) {
			return true;
		}
		break;
	case INPUT_R3:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0) {
			return true;
		}
		break;
	case INPUT_START:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0) {
			return true;
		}
		break;
	case INPUT_BACK:
		if ((state_.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0) {
			return true;
		}
		break;
	}

	return false;
}

BYTE InputGamePad::GetPreButtonPress(PadButtoms button) const {
	if (button == INPUT_R2) {
		return preState_.Gamepad.bRightTrigger >= kSinkingState_;
	} else if (button == INPUT_L2) {
		return preState_.Gamepad.bLeftTrigger >= kSinkingState_;
	}

	if (GetPreLeftStickInclination() > kLeftStickDeadZone) {
		if (button == INPUT_LSTICK_UP) {
			if (GetPreLeftStickDirection().y > 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_DOWN) {
			if (GetPreLeftStickDirection().y < 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_LEFT) {
			if (GetPreLeftStickDirection().x < 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_RIGHT) {
			if (GetPreLeftStickDirection().x > 0.0f) {
				return true;
			}
			return false;
		}
	}

	if (GetPreRightStickInclination() > kLeftStickDeadZone) {
		if (button == INPUT_LSTICK_UP) {
			if (GetPreRightStickDirection().y > 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_DOWN) {
			if (GetPreRightStickDirection().y < 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_LEFT) {
			if (GetPreRightStickDirection().x < 0.0f) {
				return true;
			}
			return false;
		} else if (button == INPUT_LSTICK_RIGHT) {
			if (GetPreRightStickDirection().x > 0.0f) {
				return true;
			}
			return false;
		}
	}

	switch (button) {
	case INPUT_A:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0) {
			return true;
		}
		break;
	case INPUT_B:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0) {
			return true;
		}
		break;
	case INPUT_X:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0) {
			return true;
		}
		break;
	case INPUT_Y:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0) {
			return true;
		}
		break;
	case INPUT_UP:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0) {
			return true;
		}
		break;
	case INPUT_DOWN:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0) {
			return true;
		}
		break;
	case INPUT_LEFT:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0) {
			return true;
		}
		break;
	case INPUT_RIGHT:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0) {
			return true;
		}
		break;
	case INPUT_L1:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0) {
			return true;
		}
		break;
	case INPUT_R1:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0) {
			return true;
		}
		break;
	case INPUT_L3:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0) {
			return true;
		}
		break;
	case INPUT_R3:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0) {
			return true;
		}
		break;
	case INPUT_START:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0) {
			return true;
		}
		break;
	case INPUT_BACK:
		if ((preState_.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0) {
			return true;
		}
		break;
	}

	return false;
}

void InputManager::Initialize() {// DirectInputの初期化.
	HRESULT result = DirectInput8Create(GameSystem::GetInstance()->GetWc().hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput_, nullptr);
	assert(SUCCEEDED(result));

	keyBoard_.Initialize(directInput_);
	gamePad_.Initialize();
}


void InputManager::Update() {
	keyBoard_.Update();
	gamePad_.Update();
}

void InputManager::OperationModeCheck(){
	if (gamePad_.IsOperationDevice()) {
		operationMode_ = OperationMode::Pad;
	}

	if (keyBoard_.IsOperationDevice()) {
		operationMode_ = OperationMode::KeyBoard;
	}
}
