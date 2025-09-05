#pragma once

#include "Include.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "DynamicMeshComponentUtility.h"
#include "Core/ObjectPath.h"

class Skeleton;
class MFBXLoader;

struct FBodyCapsuleData
{
    int32 AttachJointIndex = -1;
    float Height = 1.f;
    float Radius = 1.f;
    int32 PrimitiveID = -1;

    REFLECT_TOP(FBodyCapsuleData
        , PROPERTY(AttachJointIndex)
        , PROPERTY(Height)
        , PROPERTY(Radius)
    );
};

class ENGINE_DLL DynamicMesh : public StaticMesh
{
public:
    DynamicMesh() = default;

public:
    virtual void InitializeFromFBX(MFBXLoader& FbxLoaderm, const std::wstring& FilePath) override;

public:
    const std::vector<MAnimation>& GetAnimClips() { return _animationClipList; }
    bool getAnimationClip(const uint32 index, MAnimation& OutAnimationClip);
public:
    std::vector<MAnimation> _animationClipList;

public:
    const uint32 GetJointNum() const;
    std::vector<FJoint>& GetJoints();
    FJoint GetJoint(uint32 InIndex);
    FJoint GetJoint(const std::string& InName);
    int32 GetJointIndex(const std::string& InName);

public:
    std::shared_ptr<class MSkeleton> Skeleton = nullptr;

public:
    // 조인트에 어태치 하여 피직스를 나타내는 캡슐들을 저장
    std::vector<FBodyCapsuleData> BodyCapsuleDatas;

    REFLECT(
        DynamicMesh
        , PROPERTY(Skeleton)
        , PROPERTY(BodyCapsuleDatas)
    );
};

class MSkeleton : public MAsset
{
public:
    MSkeleton() = default;
    virtual ~MSkeleton() = default;

public:
    const uint32 GetJointNum() const;
    std::vector<FJoint>& GetJoints();
    FJoint GetJoint(uint32 InIndex);
    FJoint GetJoint(const std::string& InName);
    int32 GetJointIndex(const std::string& InName);

public:
    std::vector<FJoint> Joints;
    std::unordered_map<std::string, uint32> NameToJointIndex;

    REFLECT(MSkeleton
        , PROPERTY(Joints)
        , PROPERTY(NameToJointIndex)
    );
};