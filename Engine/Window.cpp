#include "Include.h"
#include "Window.h"
#include "WindowException.h"

LRESULT MWindow::DefaultWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
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

MWindow::MWindow()
{
    RECT rt = { 0, 0, 1920, 1080 };
    AdjustWindowRect(&rt, WS_OVERLAPPED, false);

    m_hWnd = CreateWindow(TEXT("className"), TEXT("title"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rt.right - rt.left, rt.bottom - rt.top, 0, 0, g_hInstance, 0);
    if (m_hWnd == nullptr)
        throw WINDOW_EXCEPTION(GetLastError());

    ShowWindow(m_hWnd, SW_SHOW);
}

MWindow::MWindow(const std::wstring &title, const int width, const int height, const std::wstring &className)
	: m_hWnd{ 0 }
    , Width(width)
    , Height(height)
{
	RECT rt = { 0, 0, width, height };
	AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW, false);

	m_hWnd = CreateWindow(className.c_str(), title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rt.right - rt.left, rt.bottom - rt.top, 0, 0, g_hInstance, 0);
	if (m_hWnd == nullptr)
		throw WINDOW_EXCEPTION(GetLastError());

	ShowWindow(m_hWnd, SW_SHOW);
}

MWindow::MWindow(const std::wstring& title, const int width, const int height, HWND Parent, const std::wstring& className)
    : m_hWnd{ 0 }
    , Width(width)
    , Height(height)
{
    RECT rt = { 0, 0, width, height };
    AdjustWindowRect(&rt, WS_OVERLAPPED, false);

    m_hWnd = CreateWindow(className.c_str(), title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rt.right - rt.left, rt.bottom - rt.top, Parent, 0, g_hInstance, 0);
    if (m_hWnd == nullptr)
        throw WINDOW_EXCEPTION(GetLastError());

    ShowWindow(m_hWnd, SW_SHOW);
}

MWindow::MWindow(LPCWSTR title, const int width, const int height, LPCWSTR className)
    : m_hWnd{ 0 }
    , Width(width)
    , Height(height)
{
	RECT rt = { 0, 0, width, height };
	AdjustWindowRect(&rt, WS_OVERLAPPED, false);

	m_hWnd = CreateWindow(className, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rt.right - rt.left, rt.bottom - rt.top, 0, 0, g_hInstance, 0);
	if (m_hWnd == nullptr)
		throw WINDOW_EXCEPTION(GetLastError());
	ShowWindow(m_hWnd, SW_SHOW);
}

void MWindow::SetTitle(const std::wstring title)
{
	SetWindowText(getHandle(), title.c_str());
}

HWND MWindow::getHandle() const
{
	return m_hWnd;
}

void MWindow::UpdateSize()
{
    RECT Rect = {};
    if (GetClientRect(getHandle(), &Rect) == FALSE)
    {
        return;
    }

    uint32 OldWidth = Width;
    uint32 OldHeight = Height;
    uint32 NewWidth = Rect.right - Rect.left;
    uint32 NewHeight = Rect.bottom - Rect.top;

    if (NewWidth == OldWidth && NewHeight == OldHeight)
    {
        return;
    }

    Width = NewWidth;
    Height = NewHeight;
    AspectRatio = static_cast<float>(Width) / Height;

    GetOnViewportSizeChangedDelegate().Broadcast(ID, OldWidth, OldHeight, NewWidth, NewHeight);
}

std::tuple<LONG, LONG> MWindow::GetWindowPos() const
{
    RECT Rect = {};
    if (GetWindowRect(getHandle(), &Rect) == FALSE)
    {
        return { 0, 0 };
    }

    return { Rect.left, Rect.top };
}

void MWindow::SetMousePos(int32 X, int32 Y)
{
    SetCursorPos(X, Y);
}

void MWindow::MouseCneter()
{
    RECT Rect = {};
    if (GetClientRect(getHandle(), &Rect) == FALSE)
    {
        return;
    }

    POINT LeftTop = { Rect.left, Rect.top };
    ClientToScreen(getHandle(), &LeftTop);

    SetMousePos(LeftTop.x + ((Rect.right - Rect.left) / 2), LeftTop.y + ((Rect.bottom - Rect.top) / 2));
}

bool MWindow::IsMouseInViewport() const
{
    Vec2 MousePos = GetMousePos();
    return MousePos.x >= 0.f && MousePos.x <= Width && MousePos.y >= 0.f && MousePos.y <= Height;
}

bool MWindow::IsForegorund() const
{
    return getHandle() == GetForegroundWindow();
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
