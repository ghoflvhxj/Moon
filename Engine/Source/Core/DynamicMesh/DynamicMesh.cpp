#include "DynamicMesh.h"
#include "Material.h"

void DynamicMesh::InitializeFromFBX(MFBXLoader& FbxLoader, const std::wstring& FilePath)
{
    StaticMesh::InitializeFromFBX(FbxLoader, FilePath);
    FbxLoader.LoadAnim(_animationClipList);

    for (std::shared_ptr<MMaterial>& Material : Materials)
    {
        // 디폴트 쉐이더
        Material->setShader(TEXT("TexAnimVertexShader.cso"), TEXT("TexPixelShader.cso"));
    }

    _jointList = FbxLoader._jointList;
    _jointCount = CastValue<uint32>(_jointList.size());

    _pSkeleton = std::make_shared<Skeleton>(this);
}

bool DynamicMesh::getAnimationClip(const int index, AnimationClip& OutAnimationClip)
{
    if (GetSize(_animationClipList) < index)
    {
        OutAnimationClip = _animationClipList[index];
        return true;
    }

    return false;
}

const uint32 DynamicMesh::getJointCount() const
{
    return _jointCount;
}

std::vector<FJoint>& DynamicMesh::getJoints()
{
    return _jointList;
}

Skeleton::Skeleton(DynamicMesh* dynamicMesh)
{
    for (uint32 i = 0; i < 199; ++i)
    {
        auto& trans = dynamicMesh->getJoints()[i]._position;
        _vertices.push_back({ Vec4{ trans.x, trans.y, trans.z, 1.f } });
        _vertices.back().BlendIndex[0] = i;
        _vertices.back().BlendWeight.x = 1.f;

        int32 parentIndex = dynamicMesh->getJoints()[i]._parentIndex;
        if (parentIndex != -1)
        {
            auto& parentTrans = dynamicMesh->getJoints()[parentIndex]._position;
            _vertices.push_back({ Vec4{ parentTrans.x, parentTrans.y, parentTrans.z, 1.f } });
            _vertices.back().BlendIndex[0] = parentIndex;
            _vertices.back().BlendWeight.x = 1.f;
        }
        else
        {
            _vertices.push_back({ Vec4{ trans.x, trans.y, trans.z, 1.f } });
        }
    }

    //_pVertexBuffer = std::make_shared<MVertexBuffer>(CastValue<uint32>(sizeof(Vertex)), CastValue<uint32>(_vertices.size()), _vertices.data());
    //_pMaterial = std::make_shared<MMaterial>();
    //_pMaterial->setShader(TEXT("Bone.cso"), TEXT("TexPixelShader.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵
    //_pMaterial->setTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
}
