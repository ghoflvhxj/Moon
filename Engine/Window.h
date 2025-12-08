#pragma once
#include "Include.h"
#include "Core/Object.h"
#include "Core/Delegate.h"

class ENGINE_DLL MWindow : public MObject
{
public:
	static LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	//-------------------------------------------------------------------------------------------------------
public:
    MWindow();
    explicit MWindow(const std::wstring& title, const int width, const int height, const std::wstring& className);
    explicit MWindow(const std::wstring& title, const int width, const int height, HWND Parent, const std::wstring& className);
	explicit MWindow(LPCWSTR title, const int width, const int height, LPCWSTR className);
	~MWindow() = default;

public:
    virtual void Initialize() {}
    virtual void Render() {}

public:
	void SetTitle(const std::wstring title);

public:
    void SetID(uint32 InID) { ID = InID; }
    uint32 GetID() const { return ID; }
protected:
    uint32 ID = 0;

public:
	HWND getHandle() const;
private:
	HWND m_hWnd = NULL;
    
protected:
    bool bFullScreen = false;

public:
    bool IsDisabled() const { return bDisble; }
    void Disable() { bDisble = true; }
protected:
    bool bDisble = false;

public:
    void UpdateSize();
public:
    template <class T>
    T GetWidth() const { return static_cast<T>(Width); }
    template <class T>
    T GetHeight() const { return static_cast<T>(Height); }
protected:
    uint32 Width = 1920;
    uint32 Height = 1080;

public:
    std::tuple<LONG, LONG> GetWindowPos() const;

public:
    float GetAspectRatio() const { return AspectRatio; }
protected:
    float AspectRatio = 1920.f / 1080.f;

public:
    FDelegate<void, uint32, uint32, uint32, uint32, uint32>& GetOnViewportSizeChangedDelegate() { return OnViewportSizeChanged; }
protected:
    FDelegate<void, uint32, uint32, uint32, uint32, uint32> OnViewportSizeChanged;

public:
    void SetMousePos(int32 X, int32 Y);
    void MouseCneter();
    const Vec2 GetMousePos() const
    {
        POINT MousePos;
        GetCursorPos(&MousePos);
        ScreenToClient(g_hWnd, &MousePos);

        return { static_cast<float>(MousePos.x), static_cast<float>(MousePos.y) };
    }

    bool IsMouseInViewport() const;
    bool IsForegorund() const;
public:
    REFLECT(MWindow)
};
