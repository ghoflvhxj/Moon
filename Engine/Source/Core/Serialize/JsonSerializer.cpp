#include "JsonSerializer.h"
#include "Serializable.h"

using namespace rapidjson;

MJsonSerializer::MJsonSerializer()
    : Allocator(Doc.GetAllocator())
{
    Doc.SetObject();
}

/*

// Vertcies 시리얼 라이즈

Value Pos(kArrayType);
Value UV(kArrayType);
Value Normal(kArrayType);
Value Tangent(kArrayType);
Value BiTangent(kArrayType);
Value BlendIndices(kArrayType);
Value BlendWeights(kArrayType);
for (const Vertex& Vtx : MeshData->Vertices)
{
    Vec4Serialize(Vtx.Pos, Pos);
    Vec2Serialize(Vtx.Tex0, UV);
    Vec3Serialize(Vtx.Normal, Normal);
    Vec3Serialize(Vtx.Tangent, Tangent);
    Vec3Serialize(Vtx.Binormal, BiTangent);
    VectorSerialize(Vtx.BlendIndex, BlendIndices);
    Vec4Serialize(Vtx.BlendWeight, BlendWeights);
}

std::vector<FProperty> Properties = Vertex->GetProperties();
std::vector<Value> Tests(GetSize(Properties));
for (const Vertex& Vetex : MeshData->Vertices)
{
    for(int i=0; i<GetSize(Tests); ++i)
    {
        Tests[i].PushBack(SerializeProperty(Properties[i]));
    }
}
*/

/*
    auto Vec2Serialize = [this](const Vec2& InVec, Value& JsonValue) {
        JsonValue.PushBack(InVec.x, Allocator);
        JsonValue.PushBack(InVec.y, Allocator);
        };

    auto Vec3Serialize = [this](const Vec3& InVec, Value& JsonValue) {
        JsonValue.PushBack(InVec.x, Allocator);
        JsonValue.PushBack(InVec.y, Allocator);
        JsonValue.PushBack(InVec.z, Allocator);
        };

    auto Vec4Serialize = [this](const Vec4& InVec, Value& JsonValue) {
        JsonValue.PushBack(InVec.x, Allocator);
        JsonValue.PushBack(InVec.y, Allocator);
        JsonValue.PushBack(InVec.z, Allocator);
        JsonValue.PushBack(InVec.w, Allocator);
        };

    auto ArraySerialize = [this](auto& In, Value& JsonValue) {
        for (auto& Value : In)
        {
            JsonValue.PushBack(Value, Allocator);
        }
    };


*/