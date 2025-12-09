#pragma once

#include "Editor.h"

class MActorEditor : public MEditorBase
{
public:
    MActorEditor();

public:
    virtual void Update() override;
    virtual void RenderUI() override;
    virtual void HandleObject() override;
    virtual void OnSaved() override;
    std::shared_ptr<MActor> Working;
};