#pragma once 

#include "Include.h"
#include "Serializerable.h"
#include "FBXLoader.h"
// TextureList, MaterialList 참조
#include "Vertex.h"

class MFBXLoader;
class MBoundingBox;
class MMaterial;

struct ENGINE_DLL FMeshData
{
    VertexList		Vertices;
    IndexList 		Indices;
};

class ENGINE_DLL StaticMesh
{
public:
    StaticMesh() = default;

public:
    void LoadFromFBX(const std::wstring& Path);
    void LoadFromAsset(const std::wstring& Path);
    virtual void InitializeFromFBX(MFBXLoader& FbxLoader, const std::wstring& FilePath);
public:
    const std::vector<uint32>& getGeometryLinkMaterialIndex() const;
    const std::vector<::Vec3>& GetAllVertexPosition() const;
    std::vector<TextureList>	Textures;
    std::vector<uint32>			_geometryLinkMaterialIndices;
    std::vector<Vec3>			AllVertexPosition;

public:
    const std::shared_ptr<FMeshData>& GetMeshData(const uint32 Index) const;
    const uint32 GetMeshNum() const { return GetSize(MeshDataList); }
protected:
    std::vector<std::shared_ptr<FMeshData>> MeshDataList;

public:
    MaterialList& getMaterials();
    std::shared_ptr<MMaterial> getMaterial(const uint32 index);
    const uint32 GetMaterialNum() const;
protected:
    MaterialList Materials;

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
};