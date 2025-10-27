#pragma once

#include "Window.h"

class MDynamicMeshEditWindow : public MWindow
{
public:
    MDynamicMeshEditWindow() = default;
    explicit MDynamicMeshEditWindow(const std::wstring& title, const int width, const int height, const std::wstring& className);
    explicit MDynamicMeshEditWindow(const std::wstring& title, const int width, const int height, HWND Parent, const std::wstring& className);

public:
    virtual void Render() override;

    REFLECT(MDynamicMeshEditWindow)
};