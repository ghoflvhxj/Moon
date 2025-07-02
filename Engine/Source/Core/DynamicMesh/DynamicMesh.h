#pragma once

#include "Include.h"
#include "Core/StaticMesh/StaticMesh.h"
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
    bool getAnimationClip(const int index, AnimationClip& OutAnimationClip);
    const uint32 getJointCount() const;
    std::vector<FJoint>& getJoints();
private:
    std::vector<AnimationClip> _animationClipList;
    std::vector<FJoint> _jointList;
    uint32 _jointCount;

public:
    std::shared_ptr<Skeleton> _pSkeleton = nullptr;
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