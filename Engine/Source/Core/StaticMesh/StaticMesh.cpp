#include "StaticMesh.h"

#include "FBXLoader.h"
// BoundingBox
#include "PrimitiveComponent.h"

#include "Material.h"
#include "Core/Serialize/JsonSerializer.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"

using namespace rapidjson;
using namespace DirectX;

void StaticMesh::LoadFromFBX(const std::wstring& FilePath)
{
    MFBXLoader FbxLoader;
    InitializeFromFBX(FbxLoader, FilePath);

    const std::vector<VertexList>& Vertices = FbxLoader.getVerticesList();
    const std::vector<IndexList>& Indices = FbxLoader.getIndicesList();

    // FBX를 이용해 Serialize할 데이터들을 저장
    uint32 GeometryNum = FbxLoader.GetGeometryNum();
    for (uint32 GeometryIndex = 0; GeometryIndex < GeometryNum; ++GeometryIndex)
    {
        auto MeshData = std::make_shared<FMeshData>();
        MeshData->Vertices = Vertices[GeometryIndex];
        MeshData->Indices = Indices[GeometryIndex];
        MeshDataList.push_back(MeshData);
    }

    // 중심점과 모든 버텍스 위치
    TotalVertexNum = 0;
    for (auto& MeshData : MeshDataList)
    {
        TotalVertexNum += GetSize(MeshData->Vertices);
    }

    AllVertexPosition.clear();
    AllVertexPosition.resize(TotalVertexNum);
    int32 VertexCounter = 0;
    CenterPos = { 0.f, 0.f, 0.f };
    for (auto& MeshData : MeshDataList)
    {
        for (uint32 i = 0; i < GetSize(MeshData->Vertices); ++i)
        {
            Vec4& Pos = MeshData->Vertices[i].Pos;

            AllVertexPosition[VertexCounter].x = Pos.x;
            AllVertexPosition[VertexCounter].y = Pos.y;
            AllVertexPosition[VertexCounter].z = Pos.z;

            CenterPos.x += Pos.x;
            CenterPos.y += Pos.y;
            CenterPos.z += Pos.z;

            ++VertexCounter;
        }
    }
    CenterPos.x /= CastValue<float>(TotalVertexNum);
    CenterPos.y /= CastValue<float>(TotalVertexNum);
    CenterPos.z /= CastValue<float>(TotalVertexNum);
}

void StaticMesh::LoadFromAsset(const std::wstring& Path)
{
    // 매터리얼은 임시, 이것도 불러와야 함
    auto& NewMaterial = std::make_shared<MMaterial>();
    NewMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso"));
    Materials.push_back(NewMaterial);

    auto ReadArray = [](Value& InValue, auto& OutVector /*std::vector*/) {
        for (auto& Value : InValue.GetArray())
        {
            OutVector.push_back(Value.GetInt());
        }
        };

    auto ReadArrayRange = [](Value& InValue, auto& Out, uint32 Start, uint32 Num) {
        for (uint32 i = 0; i < Num; ++i)
        {
            Out[i] = InValue.GetArray()[Start + i].GetInt();
        }
        };

    auto ReadVec4 = [](Value& InValue, Vec4& Vec, uint32 Start) {
        Vec.x = InValue.GetArray()[Start * 4].GetFloat();
        Vec.y = InValue.GetArray()[Start * 4 + 1].GetFloat();
        Vec.z = InValue.GetArray()[Start * 4 + 2].GetFloat();
        Vec.w = InValue.GetArray()[Start * 4 + 3].GetFloat();
        };

    auto ReadVec3 = [](Value& InValue, Vec3& Vec, uint32 Start) {
        Vec.x = InValue.GetArray()[Start * 3].GetFloat();
        Vec.y = InValue.GetArray()[Start * 3 + 1].GetFloat();
        Vec.z = InValue.GetArray()[Start * 3 + 2].GetFloat();
        };

    auto ReadVec2 = [](Value& InValue, Vec2& Vec, uint32 Start) {
        Vec.x = InValue.GetArray()[Start * 2].GetFloat();
        Vec.y = InValue.GetArray()[Start * 2 + 1].GetFloat();
        };

    FILE* fp = nullptr;
    fopen_s(&fp, "D:\\Git\\Moon\\Client\\test.json", "rb");

    char readBuffer[2048];
    FileReadStream frs(fp, readBuffer, sizeof(readBuffer));

    Document Doc;
    Doc.ParseStream(frs);

    if (Doc.HasMember("MeshNum"))
    {
        uint32 MeshNum = Doc["MeshNum"].GetInt();
        for (uint32 MeshCounter = 0; MeshCounter < MeshNum; ++MeshCounter)
        {
            std::string MeshName = "Mesh" + std::to_string(MeshCounter);
            if (Doc.HasMember(MeshName))
            {
                std::shared_ptr<FMeshData> NewMeshData = std::make_shared<FMeshData>();

                ReadArray(Doc[MeshName]["Indices"], NewMeshData->Indices);

                uint32 VertexNum = Doc[MeshName]["VertexNum"].GetInt();
                NewMeshData->Vertices.resize(VertexNum);
                for (uint32 VertexCounter = 0; VertexCounter < VertexNum; ++VertexCounter)
                {
                    ReadVec4(Doc[MeshName]["Pos"], NewMeshData->Vertices[VertexCounter].Pos, VertexCounter);
                    ReadVec2(Doc[MeshName]["UV"], NewMeshData->Vertices[VertexCounter].Tex0, VertexCounter);
                    ReadVec3(Doc[MeshName]["Normal"], NewMeshData->Vertices[VertexCounter].Normal, VertexCounter);
                    ReadVec3(Doc[MeshName]["Tangent"], NewMeshData->Vertices[VertexCounter].Tangent, VertexCounter);
                    ReadVec3(Doc[MeshName]["BiTangent"], NewMeshData->Vertices[VertexCounter].Binormal, VertexCounter);
                    ReadArrayRange(Doc[MeshName]["BlendIndices"], NewMeshData->Vertices[VertexCounter].BlendIndex, VertexCounter * 4, 4);
                    ReadArrayRange(Doc[MeshName]["BlendWeights"], NewMeshData->Vertices[VertexCounter].BlendWeight, VertexCounter * 4, 4);
                }

                MeshDataList.push_back(NewMeshData);
            }
        }
    }

    fclose(fp);

    // MeshData가 만들어짐
    TotalVertexNum = 0;
    for (auto& MeshData : MeshDataList)
    {
        TotalVertexNum += GetSize(MeshData->Vertices);
    }

    AllVertexPosition.clear();
    AllVertexPosition.resize(TotalVertexNum);
    int32 VertexCounter = 0;
    CenterPos = { 0.f, 0.f, 0.f };
    for (auto& MeshData : MeshDataList)
    {
        for (uint32 i = 0; i < GetSize(MeshData->Vertices); ++i)
        {
            Vec4& Pos = MeshData->Vertices[i].Pos;

            AllVertexPosition[VertexCounter].x = Pos.x;
            AllVertexPosition[VertexCounter].y = Pos.y;
            AllVertexPosition[VertexCounter].z = Pos.z;

            CenterPos.x += Pos.x;
            CenterPos.y += Pos.y;
            CenterPos.z += Pos.z;

            ++VertexCounter;
        }
    }
    CenterPos.x /= CastValue<float>(TotalVertexNum);
    CenterPos.y /= CastValue<float>(TotalVertexNum);
    CenterPos.z /= CastValue<float>(TotalVertexNum);
}

void StaticMesh::InitializeFromFBX(MFBXLoader& FbxLoader, const std::wstring& FilePath)
{
    FbxLoader.LoadMesh(FilePath);

    Textures = FbxLoader.GetTextures(); // MaterialTextures
    _geometryLinkMaterialIndices = FbxLoader.getLinkList();
    if (_geometryLinkMaterialIndices.size() == 0)
    {
        _geometryLinkMaterialIndices.emplace_back(0);
    }

    uint32 MaterialNum = std::max(1u, FbxLoader.GetMaterialNum());
    Materials.reserve(MaterialNum);
    for (uint32 MaterialIndex = 0; MaterialIndex < MaterialNum; ++MaterialIndex)
    {
        std::shared_ptr<MMaterial>& NewMaterial = std::make_shared<MMaterial>();

        if (MaterialIndex < Textures.size())
        {
            NewMaterial->setTextures(Textures[MaterialIndex]);
        }

        NewMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso"));
        Materials.push_back(NewMaterial);
    }

    // 바운딩 박스
    Vec3 Min, Max;
    FbxLoader.getBoundingBoxInfo(Min, Max);
    _pBoundingBox = std::make_shared<MBoundingBox>(Min, Max);

    // 바운딩 스피어
    float Radius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&Min) - XMLoadFloat3(&Max)));

}

const std::vector<uint32>& StaticMesh::getGeometryLinkMaterialIndex() const
{
    return _geometryLinkMaterialIndices;
}

const std::vector<::Vec3>& StaticMesh::GetAllVertexPosition() const
{
    return AllVertexPosition;
}

const std::shared_ptr<FMeshData>& StaticMesh::GetMeshData(const uint32 Index) const
{
    if (Index < GetMeshNum())
    {
        return MeshDataList[Index];
    }

    return nullptr;
}

MaterialList& StaticMesh::getMaterials()
{
    return Materials;
}

std::shared_ptr<MMaterial> StaticMesh::getMaterial(const uint32 index)
{
    return Materials[index];
}

const uint32 StaticMesh::GetMaterialNum() const
{
    return CastValue<uint32>(Materials.size());
}

std::shared_ptr<MBoundingBox> StaticMesh::GetBoundingBox()
{
    return _pBoundingBox;
}

const Vec3& StaticMesh::GetCenterPos() const
{
    return CenterPos;
}
