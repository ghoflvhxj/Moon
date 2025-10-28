#pragma once

#include "Include.h"
#include "Module/Module.h"

struct FWorldRenderInfo;

struct FInputDevice
{
    FInputDevice() = default;

    // 이동
    FInputDevice(FInputDevice&& Rhs)
    {
        _pKeyboard = Rhs._pKeyboard;
        Rhs._pKeyboard = nullptr;

        _pMouse = Rhs._pMouse;
        Rhs._pMouse = nullptr;
    }
    FInputDevice& operator=(FInputDevice&& Rhs)
    {
        _pKeyboard = Rhs._pKeyboard;
        Rhs._pKeyboard = nullptr;

        _pMouse = Rhs._pMouse;
        Rhs._pMouse = nullptr;

        return *this;
    }

    ~FInputDevice()
    {
        SafeRelease(_pKeyboard);
        SafeRelease(_pMouse);
    }

    // 키보드
    IDirectInputDevice8* _pKeyboard = nullptr;
    unsigned char _keyboardState[256] = {};
    unsigned char _prevKeyboardState[256] = {};

    // 마우스
    IDirectInputDevice8* _pMouse = nullptr;
    DIMOUSESTATE _mouseState = {};
    DIMOUSESTATE _prevMouseState = {};
};

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
	void updateKeyboard(IDirectInputDevice8* _pKeyboard, unsigned char* _prevKeyboardState, unsigned char* _keyboardState);
	void updateMouse(IDirectInputDevice8* _pMouse, DIMOUSESTATE& _mouseState, DIMOUSESTATE& _prevMouseState);

public:
	const bool keyDown(unsigned char key);
	const bool keyUp(unsigned char key);
	const bool keyPress(unsigned char key);

public:
	const bool mouseDown(const MOUSEBUTTON eMouseButton);
	const bool mouseUp(const MOUSEBUTTON eMouseButton);
	const bool mousePress(const MOUSEBUTTON eMouseButton);
	const LONG mouseMove(const EAxis eMouseAxis);
private:
	IDirectInput8 *_pDirectInput;
	//IDirectInputDevice8 *_pKeyboard;
	//IDirectInputDevice8 *_pMouse;

private:
	//unsigned char _keyboardState[256];
	//unsigned char _prevKeyboardState[256];

	//DIMOUSESTATE _mouseState;
	//DIMOUSESTATE _prevMouseState;

private:
	bool bFocused;

public:
    void AddDevice(const FWorldRenderInfo& InWorldBoundInfo);
protected:
    std::vector<FInputDevice> InputDevices;

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
	static const LONG mouseMove(const EAxis axis);
};
