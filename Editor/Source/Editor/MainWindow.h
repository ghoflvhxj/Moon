#pragma once

#include "Window.h"

class MEditorMainWindow : public MWindow
{
public:
    MEditorMainWindow() = default;
    explicit MEditorMainWindow(const std::wstring& title, const int width, const int height, const std::wstring& className);
    explicit MEditorMainWindow(const std::wstring& title, const int width, const int height, HWND Parent, const std::wstring& className);

public:
    virtual void Render() override;

    REFLECT(MEditorMainWindow)
};