#include "Include.h"
#include "Window.h"
#include "WindowException.h"

LRESULT Window::DefaultWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

Window::Window(const std::wstring &title, const int width, const int height, std::wstring &className)
	: m_hWnd{ 0 }
{
	RECT rt = { 0, 0, width, height };
	AdjustWindowRect(&rt, WS_OVERLAPPED, false);

	m_hWnd = CreateWindow(className.c_str(), title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rt.right - rt.left, rt.bottom - rt.top, 0, 0, g_hInstance, 0);
	if (m_hWnd == nullptr)
		throw WINDOW_EXCEPTION(GetLastError());

	ShowWindow(m_hWnd, SW_SHOW);
}

Window::Window(LPCWSTR title, const int width, const int height, LPCWSTR className)
	: m_hWnd{ 0 }
{
	RECT rt = { 0, 0, width, height };
	AdjustWindowRect(&rt, WS_OVERLAPPED, false);

	m_hWnd = CreateWindow(className, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rt.right - rt.left, rt.bottom - rt.top, 0, 0, g_hInstance, 0);
	if (m_hWnd == nullptr)
		throw WINDOW_EXCEPTION(GetLastError());
	ShowWindow(m_hWnd, SW_SHOW);
}

void Window::SetTitle(const std::wstring title)
{
	SetWindowText(getHandle(), title.c_str());
}

const HWND Window::getHandle() const
{
	return m_hWnd;
}

//Window::Exception::Exception(const int line, const char *file, const HRESULT hr)
//	: EngineException(line, file)
//	, m_hResult{ hr }
//{
//}
//
//const std::string Window::Exception::TranslateErrorCode(const HRESULT hr)
//{
//	char *pBuffer = nullptr;
//
//	DWORD message = FormatMessageA(
//		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
//		nullptr,
//		hr,
//		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
//		(LPSTR)&pBuffer, 0, nullptr
//	);
//
//	if (message == 0)
//	{
//		return "Unidentified error code";
//	}
//
//	std::string errorString = pBuffer;
//	LocalFree(pBuffer);
//
//	return errorString;
//}
//
//const char* Window::Exception::what() const
//{
//	std::ostringstream oss;
//	oss << GetType() << std::endl
//		<< "[Error Code]" << GetErrorCode() << std::endl
//		<< "[Description]" << GetErrorString() << std::endl
//		<< GetOriginString();
//
//	m_whatBuffer = oss.str();
//	return m_whatBuffer.c_str();
//}
//
//const char* Window::Exception::GetType() const
//{
//	return "Engine Window Exception";
//}
//
//const std::string Window::Exception::GetErrorString() const
//{
//	return TranslateErrorCode(m_hResult);
//}
//
//const HRESULT Window::Exception::GetErrorCode() const
//{
//	return m_hResult;
//}
