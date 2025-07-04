#pragma once 

#include "Include.h"
#include "Core/Serialize/Serializable.h"
// TextureList, MaterialList 참조
#include "Mesh/Mesh.h"

class MFBXLoader;
class MBoundingBox;
class MMaterial;

class ENGINE_DLL StaticMesh
{
public:
    StaticMesh() = default;

public:
    // Fbx 전달 버전
    void LoadFromFBX(const std::wstring& Path, MFBXLoader& FbxLoader);
    void LoadFromFBX(const std::wstring& Path);
    void LoadFromAsset(const std::wstring& Path);
    virtual void InitializeFromFBX(MFBXLoader& FbxLoader, const std::wstring& FilePath);
public:
    const std::vector<uint32>& getGeometryLinkMaterialIndex() const;
    const std::vector<::Vec3>& GetAllVertexPosition() const;
    //std::vector<TextureList>	Textures;
    // 각 메시의 매터리얼 인덱스
    std::vector<uint32>			UsedMaterialIndices;
    std::vector<Vec3>			AllVertexPosition;

public:
    std::shared_ptr<FMeshData> GetMeshData(const uint32 Index) const;
    const uint32 GetMeshNum() const { return GetSize(MeshDatas); }
protected:
    std::vector<std::shared_ptr<FMeshData>> MeshDatas;

public:
    MaterialList& getMaterials();
    std::shared_ptr<MMaterial> getMaterial(const uint32 index);
    const uint32 GetMaterialNum() const;
protected:
    MaterialList Materials;

public:
    std::vector<std::wstring> MaterialPaths;

public:
    const uint32 getVertexCount() const;
    uint32 TotalVertexNum = 0;

public:
    std::shared_ptr<MBoundingBox> GetBoundingBox();
private:
    std::shared_ptr<MBoundingBox> _pBoundingBox;

public:
    const Vec3& GetCenterPos() const;
private:
    Vec3 CenterPos = VEC3ZERO;

public:
    REFLECTABLE(
        REFLECT_FIELD(StaticMesh, MeshDatas),
        REFLECT_FIELD(StaticMesh, MaterialPaths),
        REFLECT_FIELD(StaticMesh, UsedMaterialIndices)
    );
};