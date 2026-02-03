#pragma once

#include "Module/Graphic/Shader/Shader.h"

class ENGINE_DLL MComputeShader : public MNewShader
{
public:
    MStructuredBuffer RWStructuredBuffer;
};