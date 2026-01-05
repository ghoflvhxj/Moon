#pragma once

#include "Window.h"

class MEditorBaseWindow : public MWindow
{
public:
    MEditorBaseWindow() = default;
    virtual ~MEditorBaseWindow();
    explicit MEditorBaseWindow(const std::wstring& title, const int width, const int height, const std::wstring& className);
    explicit MEditorBaseWindow(const std::wstring& title, const int width, const int height, HWND Parent, const std::wstring& className);

public:
    virtual void Initialize() override;
    virtual void Render() override;
    virtual void Release() override;
    virtual void ImGuiRender();
public:
    FDelegate<void>& GetOnImGuiRenderedDelegate() { return OnImGuiRendered; }
protected:
    FDelegate<void> OnImGuiRendered;

public:
    void InitImGui();
    bool IsImGuiInitialized() const { return Context != nullptr; }
    struct ImGuiContext* Context = nullptr;

    bool SetImGuiContext();

    REFLECT(MEditorBaseWindow)
};

class MEditorMainWindow : public MEditorBaseWindow
{
public:
    MEditorMainWindow() = default;
    explicit MEditorMainWindow(const std::wstring& title, const int width, const int height, const std::wstring& className);
    explicit MEditorMainWindow(const std::wstring& title, const int width, const int height, HWND Parent, const std::wstring& className);

public:
    virtual void ImGuiRender() override;

    REFLECT(MEditorMainWindow)
};