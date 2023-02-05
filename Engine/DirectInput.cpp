#include "stdafx.h"
#include "DirectInput.h"

#include "WindowException.h"

#include "Window.h"

DirectInput::DirectInput()
	: _pDirectInput{ nullptr }
	, _pKeyboard{ nullptr }
	, _pMouse{ nullptr }
	, _keyboardState{ 0, }
	, _prevKeyboardState{ 0, }
	, _mouseState{ 0, }
	, _prevMouseState{ 0, }
	, bFocused{ false }
{
	WINDOW_EXCEPTION(DirectInput8Create(g_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&_pDirectInput, nullptr))

	// 키보드
	WINDOW_EXCEPTION(_pDirectInput->CreateDevice(GUID_SysKeyboard, &_pKeyboard, nullptr))
	WINDOW_EXCEPTION(_pKeyboard->SetDataFormat(&c_dfDIKeyboard))
	WINDOW_EXCEPTION(_pKeyboard->SetCooperativeLevel(g_pMainWindow->getHandle(), DISCL_FOREGROUND | DISCL_EXCLUSIVE))
	WINDOW_EXCEPTION(_pKeyboard->Acquire())

	// 마우스
	WINDOW_EXCEPTION(_pDirectInput->CreateDevice(GUID_SysMouse, &_pMouse, nullptr))
	WINDOW_EXCEPTION(_pMouse->SetDataFormat(&c_dfDIMouse))
	WINDOW_EXCEPTION(_pMouse->SetCooperativeLevel(g_pMainWindow->getHandle(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))
	WINDOW_EXCEPTION(_pMouse->Acquire())
}

DirectInput::~DirectInput()
{
}

void DirectInput::update()
{
	bFocused = GetFocus() == NULL ? false : true;

	updateKeyboard();
	updateMouse();
}

void DirectInput::updateKeyboard()
{
	memcpy(_prevKeyboardState, _keyboardState, sizeof(unsigned char) * 256);
	HRESULT hr = _pKeyboard->GetDeviceState(sizeof(_keyboardState), static_cast<void *>(_keyboardState));

	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
			_pKeyboard->Acquire();
		else
			WINDOW_EXCEPTION(hr)
	}
}

void DirectInput::updateMouse()
{
	memcpy(&_prevMouseState, &_mouseState, sizeof(DIMOUSESTATE));
	_pMouse->GetDeviceState(sizeof(_mouseState), static_cast<void*>(&_mouseState));
}

const bool DirectInput::keyDown(unsigned char key)
{
	return bFocused && (_keyboardState[key] & 0x80) && !_prevKeyboardState[key];
}

const bool DirectInput::keyUp(unsigned char key)
{
	return bFocused &&  !_keyboardState[key];
}

const bool DirectInput::keyPress(unsigned char key)
{
	return bFocused && (_keyboardState[key] & 0x80) && _prevKeyboardState[key];
}

const bool DirectInput::mouseDown(const MOUSEBUTTON eMouseButton)
{
	return bFocused &&  (_mouseState.rgbButtons[static_cast<int>(eMouseButton)] & 0x80) && !_prevMouseState.rgbButtons[static_cast<int>(eMouseButton)];
}

const bool DirectInput::mouseUp(const MOUSEBUTTON eMouseButton)
{
	return bFocused &&  !_mouseState.rgbButtons[static_cast<int>(eMouseButton)];
}

const bool DirectInput::mousePress(const MOUSEBUTTON eMouseButton)
{
	return bFocused &&  _mouseState.rgbButtons[static_cast<int>(eMouseButton)] && _prevMouseState.rgbButtons[static_cast<int>(eMouseButton)];
}

const LONG DirectInput::mouseMove(const MOUSEAXIS eMouseAxis)
{
	return bFocused == true ? *(((LONG *)&_mouseState) + static_cast<LONG>(eMouseAxis)) : 0.0;
}

const bool keyDown(unsigned char key)
{
	return g_pDirectInput->keyDown(key);
}

const bool keyUp(unsigned char key)
{
	return g_pDirectInput->keyUp(key);
}

const bool keyPress(unsigned char key)
{
	return g_pDirectInput->keyPress(key);
}

const bool mouseDown(const MOUSEBUTTON button)
{
	return g_pDirectInput->mouseDown(button);
}

const bool mouseUp(const MOUSEBUTTON button)
{
	return g_pDirectInput->mouseUp(button);
}

const bool mousePress(const MOUSEBUTTON button)
{
	return g_pDirectInput->mousePress(button);
}

ENGINE_DLL const LONG mouseMove(const MOUSEAXIS axis)
{
	return g_pDirectInput->mouseMove(axis);
}
