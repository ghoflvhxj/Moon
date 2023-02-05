#pragma once
#ifndef __DIRECT_INPUT_H__

class ENGINE_DLL DirectInput
{
public:
	explicit DirectInput();
	~DirectInput();

public:
	void update();
private:
	void updateKeyboard();
	void updateMouse();

public:
	const bool keyDown(unsigned char key);
	const bool keyUp(unsigned char key);
	const bool keyPress(unsigned char key);

public:
	const bool mouseDown(const MOUSEBUTTON eMouseButton);
	const bool mouseUp(const MOUSEBUTTON eMouseButton);
	const bool mousePress(const MOUSEBUTTON eMouseButton);
	const LONG mouseMove(const MOUSEAXIS eMouseAxis);
private:
	IDirectInput8 *_pDirectInput;
	IDirectInputDevice8 *_pKeyboard;
	IDirectInputDevice8 *_pMouse;

private:
	unsigned char _keyboardState[256];
	unsigned char _prevKeyboardState[256];

	DIMOUSESTATE _mouseState;
	DIMOUSESTATE _prevMouseState;

private:
	bool bFocused;
};

ENGINE_DLL const bool keyDown(unsigned char key);
ENGINE_DLL const bool keyUp(unsigned char key);
ENGINE_DLL const bool keyPress(unsigned char key);
ENGINE_DLL const bool mouseDown(const MOUSEBUTTON button);
ENGINE_DLL const bool mouseUp(const MOUSEBUTTON button);
ENGINE_DLL const bool mousePress(const MOUSEBUTTON button);
ENGINE_DLL const LONG mouseMove(const MOUSEAXIS axis);

#define __DIRECT_INPUT_H__
#endif