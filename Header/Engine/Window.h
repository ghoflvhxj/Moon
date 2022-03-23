#pragma once
#ifndef __WINDOW_H__

class ENGINE_DLL Window
{
public:
	static LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	//-------------------------------------------------------------------------------------------------------
public:
	explicit Window(const std::wstring &title, const int width, const int height, std::wstring &className);
	explicit Window(LPCWSTR title, const int width, const int height, LPCWSTR className);
	~Window() = default;

public:
	void SetTitle(const std::wstring title);

public:
	const HWND getHandle() const;
private:
	HWND m_hWnd;

};

#define __WINDOW_H__
#endif