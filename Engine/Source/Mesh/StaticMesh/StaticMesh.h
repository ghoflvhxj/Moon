#pragma once 

#include "Include.h"
#include "Core/Asset.h"

// 리플렉션 등록에 필요함
#include "Material.h"
#include "Mesh/Mesh.h"
#include "Core/Physics/Physics.h"

class MFBXLoader;
class MBoundingBox;
class MMaterial;
class MPhysics;

class ENGINE_DLL MMesh : public MAsset
{
public:
    MMesh() = default;

public:
    void LoadFromFBX(const std::wstring& Path, MFBXLoader& FbxLoader);
    void LoadFromFBX(const std::wstring& Path);
    virtual void InitializeFromFBX(MFBXLoader& FbxLoader, const std::wstring& FilePath);

public:
    virtual bool Load(const std::wstring& InPath) override;
    virtual void OnLoaded() override;

public:
    virtual void SetPhysics(std::shared_ptr<MPhysics> InPhysics) {}
    virtual std::shared_ptr<MPhysics> GetPhysics() { return nullptr; }

public:
    FMeshData& GetMeshData(const uint32 Index);
    const std::vector<FMeshData>& GetMeshDatas() const { return MeshDatas; }
    const uint32 GetMeshNum() const { return GetSize(MeshDatas); }
protected:
    std::vector<FMeshData> MeshDatas;

    // 옷감 데이터
public:
    bool IsClothigMesh(uint32 InMeshIndex);
    std::vector<FClothData>& GetClothDatas() { return ClothDatas; }
protected:
    std::vector<FClothData> ClothDatas;


public:
    MaterialList& getMaterials();
    std::shared_ptr<MMaterial> getMaterial(const uint32 index);
    const uint32 GetMaterialNum() const;
protected:
    MaterialList Materials;

public:
    const std::vector<uint32>& getGeometryLinkMaterialIndex() const;
    // 각 메시의 매터리얼 인덱스
    std::vector<uint32>	UsedMaterialIndices;

public:
    const std::vector<::Vec3>& GetAllVertexPosition() const;
    std::vector<Vec3> AllVertexPosition;

public:
    const Vec3& GetCenterPos() const;
private:
    Vec3 CenterPos = VEC3ZERO;

public:
    const uint32 getVertexCount() const;
    uint32 TotalVertexNum = 0;

public:
    std::shared_ptr<MBoundingBox> GetBoundingBox();
private:
    std::shared_ptr<MBoundingBox> _pBoundingBox;

public:
    virtual void Test() {}


    REFLECT(
        MMesh,
        PROPERTY(MeshDatas),
        PROPERTY(Materials),
        PROPERTY(UsedMaterialIndices),
        PROPERTY(ClothDatas),
    )
};

class ENGINE_DLL StaticMesh : public MMesh
{
public:
    StaticMesh() = default;

    // 피직스 데이터
public:
    virtual void SetPhysics(std::shared_ptr<MPhysics> InPhysics) override { Physics = InPhysics; }
    virtual std::shared_ptr<MPhysics> GetPhysics() override { return Physics; }
protected:
    std::shared_ptr<MPhysics> Physics = nullptr;

public:
    REFLECT(
        StaticMesh,
        PROPERTY(Physics)
    )
};