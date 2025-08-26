#pragma once

#include "Module/Module.h"

class ENGINE_DLL MDirectInput : public MModule
{
public:
	explicit MDirectInput();
	virtual ~MDirectInput() = default;

public:
    virtual bool Initialize() override;
    virtual void Update() override;
    virtual void Release() override;

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

    REFLECT(MDirectInput)
};

class ENGINE_DLL InputManager
{

public:
	static const bool keyDown(unsigned char key);
	static const bool keyUp(unsigned char key);
	static const bool keyPress(unsigned char key);
	static const bool mouseDown(const MOUSEBUTTON button);
	static const bool mouseUp(const MOUSEBUTTON button);
	static const bool mousePress(const MOUSEBUTTON button);
	static const LONG mouseMove(const MOUSEAXIS axis);
};
