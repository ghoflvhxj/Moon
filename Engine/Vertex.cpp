#include "Vertex.h"

template <>
const FTypeDesc* GetTypeDesc<Graphic::VERTEX_COMMON>()
{
    static FTypeDesc NewTypeDesc = {
        nullptr,
        "VERTEX_COMMON",
        sizeof(Graphic::VERTEX_COMMON),
        {
            MakeProp("Pos", &Graphic::VERTEX_COMMON::Pos, nullptr),
            MakeProp("Color", &Graphic::VERTEX_COMMON::Color, nullptr),
            MakeProp("Tex0", &Graphic::VERTEX_COMMON::Tex0, nullptr),
            MakeProp("Normal", &Graphic::VERTEX_COMMON::Normal, nullptr),
            MakeProp("Tangent", &Graphic::VERTEX_COMMON::Tangent, nullptr),
            MakeProp("Binormal", &Graphic::VERTEX_COMMON::Binormal, nullptr),
            MakeProp("BlendIndex", &Graphic::VERTEX_COMMON::BlendIndex, nullptr),
            MakeProp("BlendWeight", &Graphic::VERTEX_COMMON::BlendWeight, nullptr),
        }
    };

    return &NewTypeDesc;
}