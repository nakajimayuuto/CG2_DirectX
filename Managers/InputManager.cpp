#include "InputManager.h"
#include "../Engine/SystemFile/GameSystem.h"
#include <algorithm>
#include <winuser.h>
#include "../Engine/SystemFile/ImGui.h"
#include "../Engine/SystemFile/DeltaTime.h"
#include "../Engine/Math/Easing.h"

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

void InputMouse::Initialize(IDirectInput8* directInput) {

	// キーボードデバイスの作成.
	HRESULT result = directInput->CreateDevice(GUID_SysMouse, &mouse_, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のリセット.
	result = mouse_->SetDataFormat(&c_dfDIMouse2); // 標準形式.
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット.
	result = mouse_->SetCooperativeLevel(
		GameSystem::GetInstance()->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));

	mousePosition_ = { 0.0f,0.0f };
	preMousePosition_ = { 0.0f,0.0f };

	mouseScreenPosition_ = { 0.0f,0.0f };
	preMouseScreenPosition_ = { 0.0f,0.0f };
	ZeroMemory(&mouseState_, sizeof(DIMOUSESTATE2));
	ZeroMemory(&preMouseState_, sizeof(DIMOUSESTATE2));

	isCursorFixed_ = false;
	isCursorVisible_ = true;

	isDebugCursorMovingAllow_ = false;

	ShowCursor(isCursorVisible_);
}

void InputMouse::Update() {
	preMouseState_ = mouseState_;
	HRESULT result = mouse_->GetDeviceState(
		sizeof(DIMOUSESTATE2),
		&mouseState_
	);

	if (FAILED(result)) {

		mouse_->Acquire();

		result = mouse_->GetDeviceState(
			sizeof(DIMOUSESTATE2),
			&mouseState_);
	}

	if ((mouseState_.rgbButtons[0] & 0x80) != 0) {
		result;
	}

	preMousePosition_ = mousePosition_;
	preMouseScreenPosition_ = mouseScreenPosition_;

	POINT pos = { 0,0 };

	GetCursorPos(&pos);
	mouseScreenPosition_ = { static_cast<float>(pos.x),static_cast<float>(pos.y) };

	ScreenToClient(GameSystem::GetInstance()->GetHWND(), &pos);
	mousePosition_ = { static_cast<float>(pos.x),static_cast<float>(pos.y) };


	if (!isDebugCursorMovingAllow_) {
		if (isCursorFixed_) {
			SetCursorPosition({ Environment::GetInstance()->GetWindowSize().width / 2.0f, Environment::GetInstance()->GetWindowSize().height / 2.0f });
		}
	}
}

void InputMouse::SetCursorPosition(Vector2 position) {
	if (isDebugCursorMovingAllow_) {
		return;
	}

	POINT pos;
	pos.x = static_cast<int>(position.x);
	pos.y = static_cast<int>(position.y);

	ClientToScreen(GameSystem::GetInstance()->GetHWND(), &pos);

	SetCursorPos(pos.x, pos.y);
}

bool InputKeyBoard::IsOperationDevice() {
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
		return { 0.0f,0.0f };
	}
	return result.Normalize();
}

float InputGamePad::GetLeftStickInclination() const {
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

void InputGamePad::SetVibration(float left, float right){
	XINPUT_VIBRATION vibration{};
	
	if (left > 1.0f) {
		left = 1.0f;
	} else if (left < 0.0f){
		left = 0.0f;
	}
	
	if (right > 1.0f) {
		right = 1.0f;
	} else if (right < 0.0f){
		right = 0.0f;
	}

	vibration.wLeftMotorSpeed = static_cast<WORD>(65535.0f * left);
	vibration.wRightMotorSpeed = static_cast<WORD>(65535.0f * right);

	XInputSetState(
		padNo_,
		&vibration
	);
}

bool InputGamePad::IsOperationDevice() {
	for (size_t i = 0; i < INPUT_MAX; i++) {
		if (GetButtonPress(static_cast<PadButtons>(i))) {
			return true;
		}
	}

	return false;
}

BYTE InputGamePad::GetButtonPress(PadButtons button) const {
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

	switch (button) {
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

BYTE InputGamePad::GetPreButtonPress(PadButtons button) const {
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
	mouse_.Initialize(directInput_);
	gamePad_.Initialize();

	isDebugCursorMovingAllow_ = false;
}


void InputManager::Update() {
	keyBoard_.Update();
	mouse_.Update();

#ifdef _DEBUG
	if (keyBoard_.TriggerKey(DIK_F1) && keyBoard_.PressKey(DIK_LSHIFT)) {
		if (isDebugCursorMovingAllow_) {
			isDebugCursorMovingAllow_ = false;
			mouse_.SetIsCursorVisible(mouse_.GetIsCursorVisible());
		} else {
			isDebugCursorMovingAllow_ = true;
		}
	}
	mouse_.SetIsDebugCursorMovingAllow(isDebugCursorMovingAllow_);
#endif // _DEBUG

	gamePad_.Update();
	VibrationUpdate();
}

void InputManager::SetVibration(float left, float right, float duration){
	if (vibrationType_ == VibrationType::CONTINUATION) {
		return;
	}

	leftVibrationMag_ = left;
	rightVibrationMag_ = right;
	vibrationTimerMax_ = duration;
	vibrationTimer_ = 0.0f;
	isVibration_ = true;
	vibrationType_ = VibrationType::FIXED_TIME;
}

void InputManager::SetContinuationVibration(float left, float right, bool isVibration){
	leftVibrationMag_ = left;
	rightVibrationMag_ = right;
	isVibration_ = isVibration;

	if (isVibration_) {
		vibrationType_ = VibrationType::CONTINUATION;
		gamePad_.SetVibration(left,right);
	} else {
		vibrationType_ = VibrationType::FIXED_TIME;
		leftVibrationMag_ = 0.0f;
		rightVibrationMag_ = 0.0f;
		gamePad_.SetVibration(0.0f,0.0f);
	}
}

bool InputManager::PressAction(InputAction action) const{
	if (IsGamePadConnect()) {
		for (auto padButton : inputActions_[static_cast<size_t>(action)].second) {
			return gamePad_.PressButton(padButton);
		}
	} else {
		for (auto key : inputActions_[static_cast<size_t>(action)].first.first) {
			return keyBoard_.PressKey(key);
		}

		for (auto mouseButton : inputActions_[static_cast<size_t>(action)].first.second) {
			return mouse_.PressMouse(mouseButton);
		}
	}
	return false;
}

bool InputManager::TriggerAction(InputAction action) const{
	if (IsGamePadConnect()) {
		for (auto padButton : inputActions_[static_cast<size_t>(action)].second) {
			return gamePad_.TriggerButton(padButton);
		}
	} else {
		for (auto key : inputActions_[static_cast<size_t>(action)].first.first) {
			return keyBoard_.TriggerKey(key);
		}

		for (auto mouseButton : inputActions_[static_cast<size_t>(action)].first.second) {
			return mouse_.TriggerMouse(mouseButton);
		}
	}
	return false;
}

bool InputManager::ReleaseAction(InputAction action) const{
	if (IsGamePadConnect()) {
		for (auto padButton : inputActions_[static_cast<size_t>(action)].second) {
			return gamePad_.ReleaseButton(padButton);
		}
	} else {
		for (auto key : inputActions_[static_cast<size_t>(action)].first.first) {
			return keyBoard_.ReleaseKey(key);
		}

		for (auto mouseButton : inputActions_[static_cast<size_t>(action)].first.second) {
			return mouse_.ReleaseMouse(mouseButton);
		}
	}
	return false;
}

bool InputManager::NoneAction(InputAction action) const{
	if (IsGamePadConnect()) {
		for (auto padButton : inputActions_[static_cast<size_t>(action)].second) {
			return gamePad_.NoneButton(padButton);
		}
	} else {
		for (auto key : inputActions_[static_cast<size_t>(action)].first.first) {
			return keyBoard_.NoneKey(key);
		}

		for (auto mouseButton : inputActions_[static_cast<size_t>(action)].first.second) {
			return mouse_.NoneMouse(mouseButton);
		}
	}
	return false;
}

void InputManager::VibrationUpdate(){
	if (!isVibration_) {
		return;
	}

	if (vibrationType_ == VibrationType::FIXED_TIME) {

		float left;
		float right;
		vibrationTimer_ += DeltaTime::GetInstance()->GetDeltaTime();
		left = Easing(leftVibrationMag_, 0.0f, vibrationTimer_, vibrationTimerMax_, EaseType::kEaseInOut);
		right = Easing(rightVibrationMag_, 0.0f, vibrationTimer_, vibrationTimerMax_, EaseType::kEaseInOut);

		if (vibrationTimer_ >= vibrationTimerMax_) {
			isVibration_ = false;
			leftVibrationMag_ = 0.0f;
			rightVibrationMag_ = 0.0f;
			gamePad_.SetVibration(0.0f,0.0f);
		} else {
			gamePad_.SetVibration(left,right);
		}

	}
}

void InputManager::OperationModeCheck() {
	if (gamePad_.IsOperationDevice()) {
		operationMode_ = OperationMode::Pad;
	}

	if (keyBoard_.IsOperationDevice()) {
		operationMode_ = OperationMode::KeyBoard;
	}
}
