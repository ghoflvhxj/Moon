#pragma once

#include "Window.h"

class MDynamicMeshEditWindow : public MWindow
{
public:
    MDynamicMeshEditWindow() = default;

public:
    virtual void Render() override;

    REFLECT(MDynamicMeshEditWindow)
};