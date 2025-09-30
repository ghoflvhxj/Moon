#include "DynamicMesh.h"
#include "FBXLoader.h"
#include "Material.h"

void DynamicMesh::InitializeFromFBX(MFBXLoader& FbxLoader, const std::wstring& FilePath)
{
    StaticMesh::InitializeFromFBX(FbxLoader, FilePath);
    FbxLoader.LoadFBXAnim(_animationClipList);

    for (std::shared_ptr<MMaterial>& Material : Materials)
    {
        // 디폴트 쉐이더
        Material->setShader(TEXT("TexAnimVertexShader.cso"), TEXT("TexPixelShader.cso"));
    }

    Skeleton = std::make_shared<MSkeleton>();
    Skeleton->SetAssetPath(FbxLoader.GetDirectory() + FbxLoader.GetFileName() + TEXT("_Skeleton.json"));
    Skeleton->Joints = FbxLoader.GetJoints();
    Skeleton->NameToJointIndex = FbxLoader.GetNameToJointIndex();
}

bool DynamicMesh::getAnimationClip(const uint32 index, MAnimation& OutAnimationClip)
{
    if (GetSize(_animationClipList) > index)
    {
        OutAnimationClip = _animationClipList[index];
        return true;
    }

    return false;
}

const uint32 DynamicMesh::GetJointNum() const
{
    return Skeleton ? Skeleton->GetJointNum() : 0;
}

std::vector<FJoint>& DynamicMesh::GetJoints()
{
    return Skeleton->GetJoints();
}

FJoint DynamicMesh::GetJoint(uint32 InIndex)
{
    return Skeleton->GetJoint(InIndex);
}

FJoint DynamicMesh::GetJoint(const std::string& InName)
{
    return Skeleton->GetJoint(InName);
}

int32 DynamicMesh::GetJointIndex(const std::string& InName)
{
    return Skeleton->GetJointIndex(InName);
}

const std::shared_ptr<MSkeleton>& DynamicMesh::GetSkeleton()
{
    return Skeleton;
}

void MSkeleton::OnLoaded()
{
    Super::OnLoaded();

    for (uint32 i = 0; i < GetSize(Joints); ++i)
    {
        const FJoint& Joint = Joints[i];
        ChildJoints[Joint._parentIndex].push_back(Joint);
    }
}

void MSkeleton::SetJoints(std::vector<FJoint>& InJoints)
{
    Joints = std::move(InJoints);

    for (uint32 i = 0; i < GetSize(Joints); ++i)
    {
        const FJoint& Joint = Joints[i];
        ChildJoints[Joint._parentIndex].push_back(Joint);
    }
}

const uint32 MSkeleton::GetJointNum() const
{
    return GetSize(Joints);
}

std::vector<FJoint>& MSkeleton::GetJoints()
{
    return Joints;
}

FJoint MSkeleton::GetJoint(uint32 InIndex)
{
    if (InIndex != -1 && InIndex < GetSize(Joints))
    {
        return Joints[InIndex];
    }

    return FJoint();
}

FJoint MSkeleton::GetJoint(const std::string& InName)
{
    return GetJoint(GetJointIndex(InName));
}

int32 MSkeleton::GetJointIndex(const std::string& InName) const
{
    auto& Iter = NameToJointIndex.find(InName);
    if (Iter != NameToJointIndex.end())
    {
        return Iter->second;
    }

    return -1;
}

const std::vector<FJoint>& MSkeleton::GetChildJoints(const std::string& InName)
{
    return GetChildJoints(GetJointIndex(InName));
}

const std::vector<FJoint>& MSkeleton::GetChildJoints(int32 InIndex)
{
    auto& Iter = ChildJoints.find(InIndex);
    if (Iter != ChildJoints.end())
    {
        return Iter->second;
    }

    return ChildJoints[-2]; // -1은 루트 조인트가 들어가있음.
}
