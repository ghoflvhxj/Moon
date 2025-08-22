#pragma once

#include "Include.h"

class ENGINE_DLL MModule
{
public:
    virtual bool Initialize() = 0;
    virtual void Update() = 0;
    virtual void Render() = 0;
};