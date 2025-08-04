#pragma once

#include "Include.h"

struct ENGINE_DLL FObjectPath
{
    std::wstring Path;

    REFLECT_TOP(
        FObjectPath,
        PROPERTY(Path)
    );
};