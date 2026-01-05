#pragma once

#include "Manager.h"

#undef FindWindow

class MWindow;

class ENGINE_DLL WindowManager : public Manager<WindowManager>
{
	using WindowMap = std::unordered_map<HWND, std::shared_ptr<MWindow>>;

	//---------------------------------------------------------------------------------------------------------------------------------
public:
	explicit WindowManager(const HINSTANCE hInstance);
	virtual ~WindowManager() = default;

public:
    void Release();

public:
	const bool AddWindowClass(const WNDCLASS &wndClass);

public:		
    template <class T>
    const std::shared_ptr<T> AddWindow(const std::wstring& InTitle, const int width, const int height, LPCWSTR className = DEFAULT_CLASSNAME)
    {
        if (T::GetTypeDescStatic()->IsA<MWindow>() == false)
        {
            return nullptr;
        }

        auto pWindow = std::make_shared<T>(InTitle.c_str(), width, height, className);
        AddWindow(pWindow);
        return pWindow;
    }

    template <class T>
    const std::shared_ptr<T> AddWindow(const std::wstring& InTitle, const int width, const int height, HWND Parent, LPCWSTR className = DEFAULT_CLASSNAME)
    {
        if (T::GetTypeDescStatic()->IsA<MWindow>() == false)
        {
            return nullptr;
        }

        auto pWindow = std::make_shared<T>(InTitle.c_str(), width, height, className);
        AddWindow(pWindow);
        return pWindow;
    }

	const std::shared_ptr<MWindow> GetWindow(const HWND hWnd);
    void FilterWindow(std::function<bool(std::shared_ptr<MWindow>)> InFilterFunc, std::vector<std::shared_ptr<MWindow>>& OutWindows);
	bool FindWindow(const HWND hWnd);
    bool AddWindow(std::shared_ptr<MWindow> pWindow);
private:
	WindowMap m_windowMap;
    uint32 WindowIDCounter = 0;

public:
	const bool SetMainWindow(const std::shared_ptr<MWindow> pWindow);
	const std::shared_ptr<MWindow> GetMainWindow();
private:
	std::shared_ptr<MWindow> m_pMainWindow;

};
