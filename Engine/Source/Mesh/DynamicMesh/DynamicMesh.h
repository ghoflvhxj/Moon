#pragma once

#include "Include.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "DynamicMeshComponentUtility.h"

class Skeleton;
class MFBXLoader;

class ENGINE_DLL DynamicMesh : public StaticMesh
{
public:
    DynamicMesh() = default;

public: //삭제예정
    virtual void InitializeFromFBX(MFBXLoader& FbxLoaderm, const std::wstring& FilePath) override;
public:
    const std::vector<AnimationClip>& GetAnimClips() { return _animationClipList; }
    bool getAnimationClip(const int index, AnimationClip& OutAnimationClip);

private:
    std::vector<AnimationClip> _animationClipList;


public:
    const uint32 GetJointNum() const;
    std::vector<FJoint>& GetJoints();
    FJoint GetJoint(uint32 InIndex);
    FJoint GetJoint(const std::string& InName);
    int32 GetJointIndex(const std::string& InName);
protected:
    std::unordered_map<std::string, uint32> NameToJointIndex;
    std::vector<FJoint> Joints;
    uint32 _jointCount = 0;

public:
    std::shared_ptr<Skeleton> _pSkeleton = nullptr;

public:
    //REFLECTABLE(
    //    DynamicMesh,
    //    REFLECT_FIELD(MeshDatas),
    //    REFLECT_FIELD(MaterialPaths),
    //    REFLECT_FIELD(UsedMaterialIndices)
    //);
};

class Skeleton
{
public:
    Skeleton(DynamicMesh* dynamicMesh);
public:
    std::vector<Vertex> _vertices;
    std::vector<Index>	_indices;

    //public:
    //	std::shared_ptr<MVertexBuffer> getVertexBuffer() { return _pVertexBuffer; }
    //	std::shared_ptr<MIndexBuffer> getIndexBuffer() { return nullptr; }
    //protected:
    //	std::shared_ptr<MVertexBuffer> _pVertexBuffer;
    //	std::shared_ptr<MIndexBuffer> _pIndexBuffer = nullptr;

public:
    std::shared_ptr<MMaterial> getMaterial() { return _pMaterial; }
protected:
    std::shared_ptr<MMaterial> _pMaterial;
};