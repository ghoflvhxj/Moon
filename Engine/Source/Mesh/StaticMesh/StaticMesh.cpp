#include "StaticMesh.h"

#include "FBXLoader.h"
// BoundingBox
#include "PrimitiveComponent.h"

#include "Material.h"
#include "Texture.h"

#include "Core/Serialize/JsonSerializer.h"
#include "Core/Serialize/JsonDeserializer.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"

using namespace rapidjson;
using namespace DirectX;

void StaticMesh::LoadFromFBX(const std::wstring& Path, MFBXLoader& FbxLoader)
{
    InitializeFromFBX(FbxLoader, Path);

    const std::vector<VertexList>& Vertices = FbxLoader.getVerticesList();
    const std::vector<IndexList>& Indices = FbxLoader.getIndicesList();

    // FBX를 이용해 Serialize할 데이터들을 저장
    uint32 GeometryNum = FbxLoader.GetGeometryNum();
    for (uint32 GeometryIndex = 0; GeometryIndex < GeometryNum; ++GeometryIndex)
    {
        auto NewMeshData = std::make_shared<FMeshData>();
        NewMeshData->Vertices = Vertices[GeometryIndex];
        NewMeshData->Indices = Indices[GeometryIndex];
        MeshDatas.push_back(NewMeshData);
    }

    // 중심점과 모든 버텍스 위치
    TotalVertexNum = 0;
    for (auto& MeshData : MeshDatas)
    {
        TotalVertexNum += GetSize(MeshData->Vertices);
    }

    AllVertexPosition.clear();
    AllVertexPosition.resize(TotalVertexNum);
    int32 VertexCounter = 0;
    CenterPos = { 0.f, 0.f, 0.f };
    for (auto& MeshData : MeshDatas)
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

void StaticMesh::LoadFromFBX(const std::wstring& FilePath)
{
    MFBXLoader FbxLoader;
    LoadFromFBX(FilePath, FbxLoader);
}

void StaticMesh::LoadFromAsset(const std::wstring& Path)
{
    // 메시 로드
    MJsonDeserializer Deserializer;
    Deserializer.Deserialize(*this, Path);

    // 매터리얼 로드
    for (auto& MaterialPath : MaterialPaths)
    {
        std::shared_ptr<MMaterial> NewMaterial = std::make_shared<MMaterial>();
        MJsonDeserializer MatDeserializer;
        MatDeserializer.Deserialize(*NewMaterial, MaterialPath);

        NewMaterial->OnLoaded();

        Materials.push_back(NewMaterial);
    }

    // 로드된 데이터로 정보 설정
    TotalVertexNum = 0;
    for (auto& MeshData : MeshDatas)
    {
        TotalVertexNum += GetSize(MeshData->Vertices);
    }

    AllVertexPosition.clear();
    AllVertexPosition.resize(TotalVertexNum);
    int32 VertexCounter = 0;
    CenterPos = { 0.f, 0.f, 0.f };
    for (auto& MeshData : MeshDatas)
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

    std::vector<TextureList>& Textures = FbxLoader.GetTextures(); // MaterialTextures
    UsedMaterialIndices = FbxLoader.GetMaterialIndices();
    if (UsedMaterialIndices.size() == 0)
    {
        UsedMaterialIndices.emplace_back(0);
    }

    std::set<uint32> UniqueMaterialIndices;
    for (uint32 MaterialIndex : UsedMaterialIndices)
    {
        UniqueMaterialIndices.emplace(MaterialIndex);
    }

    // 매터리얼 생성
    uint32 MaterialNum = GetSize(UniqueMaterialIndices);
    Materials.reserve(MaterialNum);
    for (uint32 MaterialIndex : UniqueMaterialIndices)
    {
        std::shared_ptr<MMaterial>& NewMaterial = std::make_shared<MMaterial>();

        if (MaterialIndex < Textures.size())
        {
            NewMaterial->setTextures(Textures[MaterialIndex]);
        }

        NewMaterial->SetName(FbxLoader.GetMaterialIName(MaterialIndex));
        NewMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso"));
        MaterialPaths.push_back(FbxLoader.GetDirectory() + NewMaterial->GetName() + TEXT(".json"));

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
    return UsedMaterialIndices;
}

const std::vector<::Vec3>& StaticMesh::GetAllVertexPosition() const
{
    return AllVertexPosition;
}

std::shared_ptr<FMeshData> StaticMesh::GetMeshData(const uint32 Index) const
{
    if (Index < GetMeshNum())
    {
        return MeshDatas[Index];
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
