#pragma once

#include "Editor.h"

class MActorEditor : public MEditorBase
{
public:
    MActorEditor();

public:
    virtual void SetObject(std::shared_ptr<MObject> InObject) override;
    virtual void RenderUI() override;

    std::shared_ptr<MActor> Actor;
};