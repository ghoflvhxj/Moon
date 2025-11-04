#include "DirectInput.h"

#include "MoonEngine.h"

#include "WindowException.h"
#include "Window.h"

MDirectInput::MDirectInput()
	: _pDirectInput{ nullptr }
	//, _pKeyboard{ nullptr }
	//, _pMouse{ nullptr }
	//, _keyboardState{ 0, }
	//, _prevKeyboardState{ 0, }
	//, _mouseState{ 0, }
	//, _prevMouseState{ 0, }
	, bFocused{ false }
{
}


bool MDirectInput::Initialize()
{
    Super::Initialize();

    WINDOW_EXCEPTION(DirectInput8Create(g_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&_pDirectInput, nullptr));

    GetEngine()->GetOnWorldAddedDelegate().Add(this, &MDirectInput::AddDevice);

    return true;
}

void MDirectInput::Update()
{
	bFocused = GetFocus() == NULL ? false : true;

    for (auto& InputDevice : InputDevices)
    {
        updateKeyboard(InputDevice._pKeyboard, InputDevice._prevKeyboardState, InputDevice._keyboardState);
        updateMouse(InputDevice._pMouse, InputDevice._prevMouseState, InputDevice._mouseState);
    }
}

void MDirectInput::Release()
{
    Super::Release();

    //SafeRelease(_pKeyboard);
    //SafeRelease(_pMouse);
    SafeRelease(_pDirectInput);
}

void MDirectInput::updateKeyboard(IDirectInputDevice8* _pKeyboard, unsigned char* _prevKeyboardState, unsigned char* _keyboardState)
{
	memcpy(_prevKeyboardState, _keyboardState, sizeof(unsigned char) * 256);
	HRESULT hr = _pKeyboard->GetDeviceState(sizeof(unsigned char) * 256, _keyboardState);

	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
			_pKeyboard->Acquire();
		else
			WINDOW_EXCEPTION(hr)
	}
}

void MDirectInput::updateMouse(IDirectInputDevice8* _pMouse, DIMOUSESTATE& _prevMouseState, DIMOUSESTATE& _mouseState)
{
	memcpy(&_prevMouseState, &_mouseState, sizeof(DIMOUSESTATE));
	_pMouse->GetDeviceState(sizeof(_mouseState), static_cast<void*>(&_mouseState));
}

const bool MDirectInput::keyDown(unsigned char key)
{
	return bFocused && (InputDevices[0]._keyboardState[key] & 0x80) && !InputDevices[0]._prevKeyboardState[key];
}

const bool MDirectInput::keyUp(unsigned char key)
{
	return bFocused &&  !InputDevices[0]._keyboardState[key];
}

const bool MDirectInput::keyPress(unsigned char key)
{
	return bFocused && (InputDevices[0]._keyboardState[key] & 0x80) && InputDevices[0]._prevKeyboardState[key];
}

const bool MDirectInput::mouseDown(const MOUSEBUTTON eMouseButton)
{
	return bFocused &&  (InputDevices[0]._mouseState.rgbButtons[static_cast<int>(eMouseButton)] & 0x80) && !InputDevices[0]._prevMouseState.rgbButtons[static_cast<int>(eMouseButton)];
}

const bool MDirectInput::mouseUp(const MOUSEBUTTON eMouseButton)
{
	return bFocused &&  !InputDevices[0]._mouseState.rgbButtons[static_cast<int>(eMouseButton)];
}

const bool MDirectInput::mousePress(const MOUSEBUTTON eMouseButton)
{
	return bFocused && InputDevices[0]._mouseState.rgbButtons[static_cast<int>(eMouseButton)] && InputDevices[0]._prevMouseState.rgbButtons[static_cast<int>(eMouseButton)];
}

const LONG MDirectInput::mouseMove(const EAxis eMouseAxis)
{
	return bFocused == true ? *(((LONG *)&InputDevices[0]._mouseState) + static_cast<LONG>(eMouseAxis)) : 0;
}

void MDirectInput::AddDevice(const FWorldRenderInfo& InWorldBoundInfo)
{
    auto& Window = InWorldBoundInfo.DstWindow;
    FInputDevice NewInputDevice = {};

    WINDOW_EXCEPTION(_pDirectInput->CreateDevice(GUID_SysKeyboard, &NewInputDevice._pKeyboard, nullptr));
    WINDOW_EXCEPTION(NewInputDevice._pKeyboard->SetDataFormat(&c_dfDIKeyboard));
    WINDOW_EXCEPTION(NewInputDevice._pKeyboard->SetCooperativeLevel(Window->getHandle(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
    WINDOW_EXCEPTION(NewInputDevice._pKeyboard->Acquire());

    // 마우스
    WINDOW_EXCEPTION(_pDirectInput->CreateDevice(GUID_SysMouse, &NewInputDevice._pMouse, nullptr));
    WINDOW_EXCEPTION(NewInputDevice._pMouse->SetDataFormat(&c_dfDIMouse));
    WINDOW_EXCEPTION(NewInputDevice._pMouse->SetCooperativeLevel(Window->getHandle(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE));
    WINDOW_EXCEPTION(NewInputDevice._pMouse->Acquire());

    InputDevices.push_back(std::move(NewInputDevice));
}

const bool InputManager::keyDown(unsigned char key)
{
	return g_pDirectInput->keyDown(key);
}

const bool InputManager::keyUp(unsigned char key)
{
	return g_pDirectInput->keyUp(key);
}

const bool InputManager::keyPress(unsigned char key)
{
	return g_pDirectInput->keyPress(key);
}

const bool InputManager::mouseDown(const MOUSEBUTTON button)
{
	return g_pDirectInput->mouseDown(button);
}

const bool InputManager::mouseUp(const MOUSEBUTTON button)
{
	return g_pDirectInput->mouseUp(button);
}

const bool InputManager::mousePress(const MOUSEBUTTON button)
{
	return g_pDirectInput->mousePress(button);
}

ENGINE_DLL const LONG InputManager::mouseMove(const EAxis axis)
{
	return g_pDirectInput->mouseMove(axis);
}
