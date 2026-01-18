#pragma once

#include "Window.h"

class MEditorBaseWindow : public MWindow
{
public:
    MEditorBaseWindow();
    virtual ~MEditorBaseWindow();
protected:
    class MEditor* EditorModule = nullptr;

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
    virtual void Copy() {}
    virtual void Paste() {}

public:
    void InitImGui();
    bool IsImGuiInitialized() const { return Context != nullptr; }
    bool SetImGuiContext();
protected:
    struct ImGuiContext* Context = nullptr;

    REFLECT(MEditorBaseWindow)
};

class MEditorMainWindow : public MEditorBaseWindow
{
public:
    MEditorMainWindow();
    virtual ~MEditorMainWindow() = default;

public:
    virtual void ImGuiRender() override;

public:
    virtual void Copy() override;
    virtual void Paste() override;

    REFLECT(MEditorMainWindow)
};