#pragma once

#include "RenderPass.h"

class MEditorPass : public MRenderPass
{
public:
    explicit MEditorPass();

public:
    virtual bool IsValidPrimitive(const FPrimitiveData& PrimitiveData) const override;
};