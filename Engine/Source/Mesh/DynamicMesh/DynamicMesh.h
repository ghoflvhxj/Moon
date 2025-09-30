#pragma once

#include "Include.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "DynamicMeshComponentUtility.h"
#include "Core/ObjectPath.h"

class MSkeleton;
class MFBXLoader;

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
    const std::shared_ptr<MSkeleton>& GetSkeleton();
public:
    std::shared_ptr<MSkeleton> Skeleton = nullptr;

public:
    // 조인트에 어태치 하여 피직스를 나타내는 캡슐들을 저장
    std::vector<FBodyCapsuleData> BodyCapsuleDatas;

    REFLECT(
        DynamicMesh
        , PROPERTY(Skeleton)
        , PROPERTY(BodyCapsuleDatas)
    );
};

class ENGINE_DLL MSkeleton : public MAsset
{
public:
    MSkeleton() = default;
    virtual ~MSkeleton() = default;

public:
    virtual void OnLoaded();

public:
    void SetJoints(std::vector<FJoint>& InJoints);

public:
    int32 GetJointIndex(const std::string& InName) const;
    const std::vector<FJoint>& GetChildJoints(const std::string& InName);
    const std::vector<FJoint>& GetChildJoints(int32 InIndex);

public:
    const uint32 GetJointNum() const;
    std::vector<FJoint>& GetJoints();
    FJoint GetJoint(uint32 InIndex);
    FJoint GetJoint(const std::string& InName);

public:
    std::vector<FJoint> Joints;
    std::unordered_map<std::string, uint32> NameToJointIndex;
    std::map<int32, std::vector<FJoint>> ChildJoints;

    REFLECT(MSkeleton
        , PROPERTY(Joints)
        , PROPERTY(NameToJointIndex)
    );
};