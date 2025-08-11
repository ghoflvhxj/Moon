#pragma once 

#include "Include.h"
#include "Core/Asset.h"

// 리플렉션 등록에 필요함
#include "Material.h"
#include "Mesh/Mesh.h"

class MFBXLoader;
class MBoundingBox;
class MMaterial;
class MPhysics;

class ENGINE_DLL StaticMesh : public MAsset
{
public:
    StaticMesh() = default;
    virtual ~StaticMesh() = default;

public:
    // Fbx 전달 버전
    void LoadFromFBX(const std::wstring& Path, MFBXLoader& FbxLoader);
    void LoadFromFBX(const std::wstring& Path);
    void LoadFromAsset(const std::wstring& Path);
    void OnLoaded();
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
    //std::vector<std::wstring> MaterialPaths;

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

    // 옷감 데이터
public:
    std::vector<FClothData>& GetClothDatas() { return ClothData; }
protected:
    std::vector<FClothData> ClothData;

    // 피직스 데이터, 메시 컴포너트는 이것의 사본을 생성해야 함
    std::shared_ptr<MPhysics> DefaultPhysics = nullptr;

public:
    REFLECT(
        StaticMesh,
        PROPERTY(MeshDatas),
        PROPERTY(Materials),
        PROPERTY(UsedMaterialIndices),
        PROPERTY(ClothData)
        //PROPERTY(DefaultPhysicsObject)
    )
};