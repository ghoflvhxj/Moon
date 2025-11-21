#pragma once

#include "Include.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "DynamicMeshComponentUtility.h"
#include "Core/ObjectPath.h"

#include "Module/Physics/CharacterPhysics.h"

class MSkeleton;
class MFBXLoader;

class ENGINE_DLL DynamicMesh : public MMesh
{
public:
    DynamicMesh() = default;

public:
    virtual void InitializeFromFBX(MFBXLoader& FbxLoaderm, const std::wstring& FilePath) override;
    virtual void Test() override;


public:
    const std::vector<MAnimation>& GetAnimClips() { return _animationClipList; }
    bool getAnimationClip(const uint32 index, MAnimation& OutAnimationClip);
public:
    std::vector<MAnimation> _animationClipList;

public:
    const uint32 GetJointNum() const;
    std::vector<FJoint>& GetJoints();
    const FJoint& GetJoint(uint32 InIndex);
    const FJoint& GetJoint(const std::string& InName);
    int32 GetJointIndex(const std::string& InName);

public:
    const std::shared_ptr<MSkeleton>& GetSkeleton();
public:
    std::shared_ptr<MSkeleton> Skeleton = nullptr;

public:
    virtual void SetPhysics(std::shared_ptr<MPhysics> InPhysics) override;
    std::shared_ptr<MPhysics> GetPhysics();
protected:
    std::shared_ptr<MDynamicMeshPhysics> Physics = nullptr;

public:
    // 조인트에 어태치 하여 피직스를 나타내는 캡슐들을 저장
    std::vector<FBodyCapsuleData> BodyCapsuleDatas;

    REFLECT(
        DynamicMesh
        , PROPERTY(Skeleton)
        , PROPERTY(BodyCapsuleDatas)
        , PROPERTY(Physics)
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
    const FJoint& GetJoint(uint32 InIndex);
    const FJoint& GetJoint(const std::string& InName);

public:
    std::vector<FJoint> Joints;
    std::unordered_map<std::string, uint32> NameToJointIndex;
    std::map<int32, std::vector<FJoint>> ChildJoints;

    REFLECT(MSkeleton
        , PROPERTY(Joints)
        , PROPERTY(NameToJointIndex)
    );
};