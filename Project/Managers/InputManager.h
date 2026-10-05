#pragma once

#define DIRECTINPUT_VERSION	0x0800 // DirectInputのバージョン指定.
#include <dinput.h>
#include <cstdint>
#include <xinput.h>
#include <array>
#include "../Engine/Math/Vector2.h"
#include "../Engine/Math/Easing.h"

enum class OperationMode {
	KeyBoard,
	Pad,
};
enum class VibrationType {
	FIXED_TIME,
	CONTINUATION,
};
/*
enum PadButtoms {
	INPUT_A = XINPUT_GAMEPAD_A,
	INPUT_B = XINPUT_GAMEPAD_B,
	INPUT_X = XINPUT_GAMEPAD_X,
	INPUT_Y = XINPUT_GAMEPAD_Y,

	INPUT_UP = XINPUT_GAMEPAD_DPAD_UP,
	INPUT_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
	INPUT_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
	INPUT_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,

	INPUT_LSTICK_UP,
	INPUT_LSTICK_DOWN,
	INPUT_LSTICK_LEFT,
	INPUT_LSTICK_RIGHT,

	INPUT_RSTICK_UP,
	INPUT_RSTICK_DOWN,
	INPUT_RSTICK_LEFT,
	INPUT_RSTICK_RIGHT = 111,

	INPUT_L1 = XINPUT_GAMEPAD_LEFT_SHOULDER,
	INPUT_R1 = XINPUT_GAMEPAD_RIGHT_SHOULDER,
	INPUT_L2,
	INPUT_R2,
	INPUT_L3 = XINPUT_GAMEPAD_LEFT_THUMB, // Lスティック押し込み.
	INPUT_R3 = XINPUT_GAMEPAD_RIGHT_THUMB, // Rスティック押し込み.

	INPUT_START = XINPUT_GAMEPAD_START,
	INPUT_BACK = XINPUT_GAMEPAD_BACK,
};
*/

enum PadButtons {
	INPUT_A, // Aボタン.
	INPUT_B, // Bボタン.
	INPUT_X, // Xボタン.
	INPUT_Y, // Yボタン.

	INPUT_UP, // 十字キー上.
	INPUT_DOWN, // 十字キー下.
	INPUT_LEFT, // 十字キー左.
	INPUT_RIGHT, // 十字キー右.

	INPUT_LSTICK_UP, 
	INPUT_LSTICK_DOWN, 
	INPUT_LSTICK_LEFT, 
	INPUT_LSTICK_RIGHT, 

	INPUT_RSTICK_UP, 
	INPUT_RSTICK_DOWN, 
	INPUT_RSTICK_LEFT, 
	INPUT_RSTICK_RIGHT, 

	INPUT_L1, // LBボタン.
	INPUT_R1, // RBボタン.
	INPUT_L2, // LTボタン.
	INPUT_R2, // RTボタン.
	INPUT_L3, // Lスティック押し込み.
	INPUT_R3, // Rスティック押し込み.

	INPUT_START,
	INPUT_BACK,

	INPUT_MAX, // パッドのEnumの最大数.
	INPUT_NONE, // パッドのEnumの最大数.
};

enum MouseButtons {
	MOUSE_LEFT, // 左クリック.
	MOUSE_RIGHT, // 右クリック.
	MOUSE_MIDDLE, // ホイールクリック.
	MOUSE_BACK, // 一つ戻る(拡張ボタン1).
	MOUSE_NEXT, // 一つ進む(拡張ボタン2).
	MOUSE_3, // 拡張ボタン3.
	MOUSE_4, // 拡張ボタン4.
	MOUSE_5, // 拡張ボタン5.

	MOUSE_MAX, // マウスのEnumの最大数.
	MOUSE_NONE, // マウスのEnumの最大数.
};

class InputKeyBoard {
public:
	void Initialize(IDirectInput8* directInput);

	void Update();

	bool PressKey(uint8_t inputKey) const { return keys_[inputKey]; };
	bool TriggerKey(uint8_t inputKey) const { return keys_[inputKey] && !preKeys_[inputKey]; };
	bool ReleaseKey(uint8_t inputKey) const { return !keys_[inputKey] && preKeys_[inputKey]; };
	bool NoneKey(uint8_t inputKey) const { return !keys_[inputKey] && !preKeys_[inputKey]; };

	bool IsOperationDevice();
private:

	IDirectInputDevice8* keyBoard_ = nullptr;

	BYTE keys_[256] = {};
	BYTE preKeys_[256] = {};
};	

class InputGamePad {
public:
	void Initialize();
	void Update();

	bool IsConnected() const;

	bool PressButton(PadButtons button) const { return GetButtonPress(button); };
	bool TriggerButton(PadButtons button) const{ return (GetButtonPress(button) && !GetPreButtonPress(button)); };
	bool ReleaseButton(PadButtons button) const{ return !(GetButtonPress(button) && GetPreButtonPress(button));};
	bool NoneButton(PadButtons button) const{ return !(GetButtonPress(button) && !GetPreButtonPress(button));};

	// Lスティックの向き.
	Vector2 GetLeftStickDirection() const;
	// Rスティックの向き.
	Vector2 GetRightStickDirection() const;

	// 前のフレームのLスティックの向き.
	Vector2 GetPreLeftStickDirection() const;
	// 前のフレームのRスティックの向き.
	Vector2 GetPreRightStickDirection() const;

	// Lスティックの傾きの強さ.
	float GetLeftStickInclination() const;
	// Rスティックの傾きの強さ.
	float GetRightStickInclination() const;

	// 前のフレームのLスティックの傾きの強さ.
	float GetPreLeftStickInclination() const;
	// 前のフレームのRスティックの傾きの強さ.
	float GetPreRightStickInclination() const;

	// L2がどれくらい押し込まれたか(0.0f～1.0f).
	float GetLeftTrigger() const { return static_cast<float>(state_.Gamepad.bLeftTrigger) / 255.0f; };
	// R2がどれくらい押し込まれたか(0.0f～1.0f).
	float GetRightTrigger() const { return static_cast<float>(state_.Gamepad.bRightTrigger) / 255.0f; };

	// 前のフレームのL2がどれくらい押し込まれたか(0.0f～1.0f).
	float GetPreLeftTrigger() const { return static_cast<float>(preState_.Gamepad.bLeftTrigger) / 255.0f; };
	//前のフレームの R2がどれくらい押し込まれたか(0.0f～1.0f).
	float GetPreRightTrigger() const { return static_cast<float>(preState_.Gamepad.bRightTrigger) / 255.0f; };

	XINPUT_STATE GetXInputState() { return state_; };
	
	XINPUT_STATE GetPreXInputState() { return preState_; };

	void SetVibration(float left, float right);

	bool IsOperationDevice();
private:
	BYTE GetButtonPress(PadButtons button)const;
	BYTE GetPreButtonPress(PadButtons button)const;
private:
	XINPUT_STATE state_{};
	XINPUT_STATE preState_{};

	DWORD padNo_ = 0;
	bool isConnected_ = false;

	static inline uint32_t kSinkingState_ = 30;

	static inline float kLeftStickDeadZone = 0.3f;
	static inline float kRightStickDeadZone = 0.3f;
};

class InputMouse {
public:
	void Initialize(IDirectInput8* directInput);

	void Update();

	bool PressMouse(MouseButtons button) const { return (mouseState_.rgbButtons[static_cast<uint32_t>(button)] & 0x80) != 0; };
	bool TriggerMouse(MouseButtons button) const {return ((mouseState_.rgbButtons[static_cast<uint32_t>(button)] & 0x80) != 0) && !((preMouseState_.rgbButtons[static_cast<uint32_t>(button)] & 0x80) != 0); };
	bool ReleaseMouse(MouseButtons button) const { return !((mouseState_.rgbButtons[static_cast<uint32_t>(button)] & 0x80) != 0) && !((preMouseState_.rgbButtons[static_cast<uint32_t>(button)] & 0x80) != 0); };
	bool NoneMouse(MouseButtons button) const { return !((mouseState_.rgbButtons[static_cast<uint32_t>(button)] & 0x80) != 0) && !((preMouseState_.rgbButtons[static_cast<uint32_t>(button)] & 0x80) != 0); };

	Vector2 GetMove() const { return { static_cast<float>(mouseState_.lX),static_cast<float>(mouseState_.lY) }; }
	float GetMoveLength() const { return GetMove().Length(); }

	Vector2 GetPos() const { return mousePosition_; }
	Vector2 GetPrePos() const { return preMousePosition_; }
	Vector2 GetScreenPos() const { return mouseScreenPosition_; }
	Vector2 GetPreScreenPos() const { return preMouseScreenPosition_; }

	void SetCursorPosition(Vector2 position);
	
	void SetIsCursorFixed(bool isCursorFixed) { isCursorFixed_ = isCursorFixed; }

	void SetIsDebugCursorMovingAllow(bool isDebugCursorMovingAllow) { isDebugCursorMovingAllow_ = isDebugCursorMovingAllow; }

	bool GetIsCursorVisible() { return isCursorVisible_; }

	void SetIsCursorVisible(bool isCursorVisible) {
		if (isDebugCursorMovingAllow_) {
			ShowCursor(true);
			return;
		}
		
		isCursorVisible_ = isCursorVisible; ShowCursor(isCursorVisible_);}
	
	float GetWheel() const {return static_cast<float>(mouseState_.lZ);}
private:
	bool isCursorFixed_;
	bool isCursorVisible_;

	bool isDebugCursorMovingAllow_;
	IDirectInputDevice8* mouse_ = nullptr;

	DIMOUSESTATE2 mouseState_ = {};
	DIMOUSESTATE2 preMouseState_ = {};

	Vector2 mousePosition_;
	Vector2 preMousePosition_;

	Vector2 mouseScreenPosition_;
	Vector2 preMouseScreenPosition_;
};

enum class InputAction {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	JUMP,
	ATTACK,
	COUNT,
};

class InputManager {
public:
	static InputManager* GetInstance();

	void Initialize();

	void Update();

	bool PressKey(uint8_t inputKey) const { return keyBoard_.PressKey(inputKey); };
	bool TriggerKey(uint8_t inputKey) const { return keyBoard_.TriggerKey(inputKey); };
	bool ReleaseKey(uint8_t inputKey) const { return keyBoard_.ReleaseKey(inputKey); };
	bool NoneKey(uint8_t inputKey) const { return keyBoard_.NoneKey(inputKey); };

	bool PressPadButton(PadButtons button) const { return gamePad_.PressButton(button); };
	bool TriggerPadButton(PadButtons button) const { return gamePad_.TriggerButton(button); };
	bool ReleasePadButton(PadButtons button) const { return gamePad_.ReleaseButton(button); };
	bool NonePadButton(PadButtons button) const { return gamePad_.NoneButton(button); };

	bool PressMouse(MouseButtons button) const { return mouse_.PressMouse(button); };
	bool TriggerMouse(MouseButtons button) const { return mouse_.TriggerMouse(button); };
	bool ReleaseMouse(MouseButtons button) const { return mouse_.ReleaseMouse(button); };
	bool NoneMouse(MouseButtons button) const { return mouse_.NoneMouse(button);};

	Vector2 GetMousePos() const { return mouse_.GetPos(); }

	float GetMouseWheel() const { return mouse_.GetWheel();}

	// Lスティックの向き.
	Vector2 GetLeftStickDirection() const { return gamePad_.GetLeftStickDirection(); };
	// Rスティックの向き.
	Vector2 GetRightStickDirection() const { return gamePad_.GetRightStickDirection(); };

	// Lスティックの傾きの強さ.
	float GetLeftStickInclination() const { return gamePad_.GetLeftStickInclination(); };
	// Rスティックの傾きの強さ.
	float GetRightStickInclination() const { return gamePad_.GetRightStickInclination(); };

	// L2がどれくらい押し込まれたか(0.0f～1.0f).
	float GetLeftTrigger() const { return gamePad_.GetLeftTrigger(); };
	// R2がどれくらい押し込まれたか(0.0f～1.0f).
	float GetRightTrigger() const { return gamePad_.GetRightTrigger();};

	InputKeyBoard GetKeyBoard() { return keyBoard_; }
	InputGamePad GetGamePad() { return gamePad_; }
	InputMouse GetMouse() { return mouse_; }

	bool IsGamePadConnect() const { return gamePad_.IsConnected(); };

	void SetIsCursorFixed(bool isCursorFixed) {mouse_.SetIsCursorFixed(isCursorFixed); }

	void SetIsCursorVisible(bool isCursorVisible) { mouse_.SetIsCursorVisible(isCursorVisible); }

	void SetVibration(float left, float right,float duration);

	void SetContinuationVibration(float left,float right,bool isVibration);
public:
	bool PressAction(InputAction action) const;
	bool TriggerAction(InputAction action) const;
	bool ReleaseAction(InputAction action) const;
	bool NoneAction(InputAction action) const;
	Vector2 GetActionDirection() const;
	
	void SetAction(InputAction action, uint8_t key, MouseButtons mouse, PadButtons pad);
private:
	void SetActions();

	void ResetActions();

	void VibrationUpdate();

	void OperationModeCheck();
private:
	std::array<std::pair<std::pair<std::vector<uint8_t>, std::vector<MouseButtons>>, std::vector<PadButtons>>, static_cast<size_t>(InputAction::COUNT)> inputActions_;
	bool isDebugCursorMovingAllow_ = false;
	IDirectInput8* directInput_ = nullptr;
	InputKeyBoard keyBoard_;
	InputMouse mouse_;
	InputGamePad gamePad_;
	OperationMode operationMode_ = OperationMode::KeyBoard;

	float leftVibrationMag_;
	float rightVibrationMag_;
	float vibrationTimer_;
	float vibrationTimerMax_;
	bool isVibration_; // シェイクしているかどうか.
	VibrationType vibrationType_; // シェイクのタイプ.
};

