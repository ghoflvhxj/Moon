#pragma once
#ifndef __WINDOW_H__

class ENGINE_DLL MWindow
{
public:
	static LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	//-------------------------------------------------------------------------------------------------------
public:
	explicit MWindow(const std::wstring &title, const int width, const int height, std::wstring &className);
	explicit MWindow(LPCWSTR title, const int width, const int height, LPCWSTR className);
	~MWindow() = default;

public:
	void SetTitle(const std::wstring title);

public:
	const HWND getHandle() const;
private:
	HWND m_hWnd;

public:
    const Vec2 GetMousePos() const
    {
        POINT MousePos;
        GetCursorPos(&MousePos);
        ScreenToClient(g_hWnd, &MousePos);

        return { static_cast<float>(MousePos.x), static_cast<float>(MousePos.y) };
    }

};

#define __WINDOW_H__
#endif