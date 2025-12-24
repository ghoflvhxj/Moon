#include "Mesh.h"
#include "FBXSDK/fbxsdk.h"

using namespace DirectX;

const FJoint FJoint::Empty = {};

void MAnimation::SetFrameInfo(uint32 InFrameRate, float InDuration, uint32 InStart, uint32 InEnd)
{
    StartFrame = InStart;
    EndFrame = InEnd;
    Duration = InDuration;
    FrameRate = InFrameRate;
    /*******************
    E: 1,   S: 0    ->  F: [0 ~ 1],     T: 2
    E: 15,  S: 13   ->  F: [13 ~ 15],   T: 3
    *******************/
    TotalFrame = EndFrame - StartFrame + 1;
    KeyFrames.resize(TotalFrame);
}

void Mesh::MakeSphere(FMeshData& OutMeshData, uint32 InSegment /*= 16*/)
{
    OutMeshData.Vertices.clear();
    OutMeshData.Indices.clear();

    uint32 Offset = 0;
    float Unit = PI2 / static_cast<float>(InSegment);
    // XZ
    for (uint32 i = 1; i <= 16; ++i)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = std::sin(Unit * i);
        NewVertex.Pos.y = 0.f;
        NewVertex.Pos.z = std::cos(Unit * i);

        OutMeshData.Vertices.push_back(NewVertex);
    }
    for (uint32 i = 1; i < InSegment; ++i)
    {
        OutMeshData.Indices.push_back(Offset + i);
        OutMeshData.Indices.push_back(Offset + i - 1);
    }
    OutMeshData.Indices.push_back(GetSize(OutMeshData.Vertices) - 1);
    OutMeshData.Indices.push_back(Offset);

    // YZ
    Offset = GetSize(OutMeshData.Vertices);
    for (uint32 i = 1; i <= InSegment; ++i)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = 0.f;
        NewVertex.Pos.y = std::cos(Unit * i);
        NewVertex.Pos.z = std::sin(Unit * i);

        OutMeshData.Vertices.push_back(NewVertex);
    }
    for (uint32 i = 1; i < InSegment; ++i)
    {
        OutMeshData.Indices.push_back(Offset + i);
        OutMeshData.Indices.push_back(Offset + i - 1);
    }
    OutMeshData.Indices.push_back(GetSize(OutMeshData.Vertices) - 1);
    OutMeshData.Indices.push_back(Offset);

    // XY
    Offset = GetSize(OutMeshData.Vertices);
    for (uint32 i = 1; i <= InSegment; ++i)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = std::sin(Unit * i);
        NewVertex.Pos.y = std::cos(Unit * i);
        NewVertex.Pos.z = 0.f;

        OutMeshData.Vertices.push_back(NewVertex);
    }
    for (uint32 i = 1; i < InSegment; ++i)
    {
        OutMeshData.Indices.push_back(Offset + i);
        OutMeshData.Indices.push_back(Offset + i - 1);
    }
    OutMeshData.Indices.push_back(GetSize(OutMeshData.Vertices) - 1);
    OutMeshData.Indices.push_back(Offset);
}

void Mesh::MakeCoordinate(FMeshData& OutMeshData)
{
    OutMeshData.Vertices.clear();
    OutMeshData.Indices.clear();

    auto AddVertex = [&](const Vec3& InPos, const XMVECTORF32& InColor) {
        Vertex NewVertex = {};
        NewVertex.Pos = { InPos.x, InPos.y, InPos.z, 1.f };
        XMStoreFloat4(&NewVertex.Color, InColor);

        OutMeshData.Vertices.push_back(NewVertex);
    };

    AddVertex(VEC3ZERO, EngineColors::Red);
    AddVertex(VEC3RIGHT, EngineColors::Red);
    AddVertex(VEC3ZERO, EngineColors::Green);
    AddVertex(VEC3UP, EngineColors::Green);
    AddVertex(VEC3ZERO, EngineColors::Blue);
    AddVertex(VEC3FORWARD, EngineColors::Blue);
}

void Mesh::MakeCapsule(FMeshData& OutMeshData, float InHalfHeight, float InRadius)
{
    OutMeshData.Vertices.clear();
    OutMeshData.Indices.clear();

    // 상단 구 XZ
    uint32 Offset = 0;
    for (uint32 i = 1; i <= 16; ++i)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = InRadius * std::sin((2.f * PI / 16.f) * i);
        NewVertex.Pos.y = (InHalfHeight);
        NewVertex.Pos.z = InRadius * std::cos((2.f * PI / 16.f) * i);
        XMStoreFloat4(&NewVertex.Color, EngineColors::Green);
        OutMeshData.Vertices.push_back(NewVertex);
    }
    for (uint32 i = 1; i < 16; ++i)
    {
        OutMeshData.Indices.push_back(Offset + i);
        OutMeshData.Indices.push_back(Offset + i - 1);
    }
    OutMeshData.Indices.push_back(GetSize(OutMeshData.Vertices) - 1);
    OutMeshData.Indices.push_back(Offset);

    // 상단 구 YZ
    Offset = GetSize(OutMeshData.Vertices);
    for (uint32 i = 1; i <= 16; ++i)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = 0.f;
        NewVertex.Pos.y = (InHalfHeight)+InRadius * std::cos((2.f * PI / 16.f) * i);
        NewVertex.Pos.z = InRadius * std::sin((2.f * PI / 16.f) * i);
        XMStoreFloat4(&NewVertex.Color, EngineColors::Green);
        OutMeshData.Vertices.push_back(NewVertex);
    }
    for (uint32 i = 1; i < 16; ++i)
    {
        OutMeshData.Indices.push_back(Offset + i);
        OutMeshData.Indices.push_back(Offset + i - 1);
    }
    OutMeshData.Indices.push_back(GetSize(OutMeshData.Vertices) - 1);
    OutMeshData.Indices.push_back(Offset);

    // 상단 구 XY
    Offset = GetSize(OutMeshData.Vertices);
    for (uint32 i = 1; i <= 16; ++i)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = InRadius * std::sin((2.f * PI / 16.f) * i);
        NewVertex.Pos.y = (InHalfHeight)+InRadius * std::cos((2.f * PI / 16.f) * i);
        NewVertex.Pos.z = 0.f;
        XMStoreFloat4(&NewVertex.Color, EngineColors::Green);
        OutMeshData.Vertices.push_back(NewVertex);
    }
    for (uint32 i = 1; i < 16; ++i)
    {
        OutMeshData.Indices.push_back(Offset + i);
        OutMeshData.Indices.push_back(Offset + i - 1);
    }
    OutMeshData.Indices.push_back(GetSize(OutMeshData.Vertices) - 1);
    OutMeshData.Indices.push_back(Offset);

    // 실린더
    Offset = GetSize(OutMeshData.Vertices);
    for (uint32 i = 1; i <= 16; ++i)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = InRadius * std::sin((2.f * PI / 16.f) * i);
        NewVertex.Pos.y = InHalfHeight;
        NewVertex.Pos.z = InRadius * std::cos((2.f * PI / 16.f) * i);
        OutMeshData.Vertices.push_back(NewVertex);

        NewVertex.Pos.y = -InHalfHeight;
        OutMeshData.Vertices.push_back(NewVertex);
    }
    for (uint32 i = 1; i < 32; i += 2)
    {
        OutMeshData.Indices.push_back(Offset + i);
        OutMeshData.Indices.push_back(Offset + i - 1);
    }

    // 하단 구 XZ
    Offset = GetSize(OutMeshData.Vertices);
    for (uint32 i = 1; i <= 16; ++i)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = InRadius * std::sin((2.f * PI / 16.f) * i);
        NewVertex.Pos.y = (-InHalfHeight);
        NewVertex.Pos.z = InRadius * std::cos((2.f * PI / 16.f) * i);
        OutMeshData.Vertices.push_back(NewVertex);
    }
    for (uint32 i = 1; i < 16; ++i)
    {
        OutMeshData.Indices.push_back(Offset + i);
        OutMeshData.Indices.push_back(Offset + i - 1);
    }
    OutMeshData.Indices.push_back(GetSize(OutMeshData.Vertices) - 1);
    OutMeshData.Indices.push_back(Offset);

    // 하단 구 YZ
    Offset = GetSize(OutMeshData.Vertices);
    for (uint32 i = 1; i <= 16; ++i)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = 0.f;
        NewVertex.Pos.y = (-InHalfHeight) + InRadius * std::cos((2.f * PI / 16.f) * i);
        NewVertex.Pos.z = InRadius * std::sin((2.f * PI / 16.f) * i);
        OutMeshData.Vertices.push_back(NewVertex);
    }
    for (uint32 i = 1; i < 16; ++i)
    {
        OutMeshData.Indices.push_back(Offset + i);
        OutMeshData.Indices.push_back(Offset + i - 1);
    }
    OutMeshData.Indices.push_back(GetSize(OutMeshData.Vertices) - 1);
    OutMeshData.Indices.push_back(Offset);

    // 하단 구 XY
    Offset = GetSize(OutMeshData.Vertices);
    for (uint32 i = 1; i <= 16; ++i)
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = InRadius * std::sin((2.f * PI / 16.f) * i);
        NewVertex.Pos.y = (-InHalfHeight) + InRadius * std::cos((2.f * PI / 16.f) * i);
        NewVertex.Pos.z = 0.f;
        OutMeshData.Vertices.push_back(NewVertex);
    }
    for (uint32 i = 1; i < 16; ++i)
    {
        OutMeshData.Indices.push_back(Offset + i);
        OutMeshData.Indices.push_back(Offset + i - 1);
    }
    OutMeshData.Indices.push_back(GetSize(OutMeshData.Vertices) - 1);
    OutMeshData.Indices.push_back(Offset);
}

void Mesh::MakeRect(FMeshData& OutMeshData)
{
    OutMeshData.Vertices.clear();
    OutMeshData.Indices.clear();

    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = -0.5f;
        NewVertex.Pos.y = 0.5f;
        NewVertex.Pos.z = 1.f;
        NewVertex.Tex0 = { 0.f, 0.f };
        OutMeshData.Vertices.push_back(NewVertex);
    }
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = 0.5f;
        NewVertex.Pos.y = 0.5f;
        NewVertex.Pos.z = 1.f;
        NewVertex.Tex0 = { 1.f, 0.f };
        OutMeshData.Vertices.push_back(NewVertex);
    }
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = -0.5f;
        NewVertex.Pos.y = -0.5f;
        NewVertex.Pos.z = 1.f;
        NewVertex.Tex0 = { 0.f, 1.f };
        OutMeshData.Vertices.push_back(NewVertex);
    }
    {
        Vertex NewVertex = {};
        NewVertex.Pos.x = 0.5f;
        NewVertex.Pos.y = -0.5f;
        NewVertex.Pos.z = 1.f;
        NewVertex.Tex0 = { 1.f, 1.f };
        OutMeshData.Vertices.push_back(NewVertex);
    }

    OutMeshData.Indices.push_back(0);
    OutMeshData.Indices.push_back(1);
    OutMeshData.Indices.push_back(2);
    OutMeshData.Indices.push_back(2);
    OutMeshData.Indices.push_back(1);
    OutMeshData.Indices.push_back(3);
}
