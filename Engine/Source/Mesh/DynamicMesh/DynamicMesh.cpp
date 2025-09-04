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
    if (InIndex < GetSize(Joints))
    {
        return Joints[InIndex];
    }

    return FJoint();
}

FJoint MSkeleton::GetJoint(const std::string& InName)
{
    if (NameToJointIndex.find(InName) != NameToJointIndex.end())
    {
        return GetJoint(NameToJointIndex[InName]);
    }

    return FJoint();
}

int32 MSkeleton::GetJointIndex(const std::string& InName)
{
    if (NameToJointIndex.find(InName) != NameToJointIndex.end())
    {
        return NameToJointIndex[InName];
    }

    return -1;
}