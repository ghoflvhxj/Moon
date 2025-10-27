#include "DynamicMeshEditWindow.h"

MDynamicMeshEditWindow::MDynamicMeshEditWindow(const std::wstring& title, const int width, const int height, const std::wstring& className)
    : Super(title, width, height, className)
{
}

MDynamicMeshEditWindow::MDynamicMeshEditWindow(const std::wstring& title, const int width, const int height, HWND Parent, const std::wstring& className)
    : Super(title, width, height, Parent, className)
{
}

void MDynamicMeshEditWindow::Render()
{

}
