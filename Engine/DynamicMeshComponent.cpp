#include "DynamicMeshComponent.h"
#include "DynamicMeshComponentUtility.h"

#include "Render.h"
#include "GraphicDevice.h"
#include "Material.h"
#include "Texture.h"

#include "Mesh/DynamicMesh/DynamicMesh.h"

#include "MainGame.h"
#include "Camera.h"

using namespace DirectX;

DynamicMeshComponent::DynamicMeshComponent()
	: MMeshComponent()
{
    Mesh = std::make_shared<DynamicMesh>();
}

DynamicMeshComponent::DynamicMeshComponent(const std::wstring& FilePath)
	: MMeshComponent()
{
	Mesh = std::make_shared<DynamicMesh>();
    SetMesh(FilePath);
}

DynamicMeshComponent::~DynamicMeshComponent()
{
}

void DynamicMeshComponent::Update(const Time deltaTime)
{
    MMeshComponent::Update(deltaTime);

    if (IsAnimPlaying())
    {
        playAnimation(_currentAinmClipIndex, deltaTime);
    }
}

const bool DynamicMeshComponent::GetPrimitiveData(std::vector<FPrimitiveData> & PrimitiveDataList)
{
	if (nullptr == Mesh)
	{
		return false;
	}

    std::shared_ptr<DynamicMesh> dMesh = GetDynamicMesh();

	uint32 geometryCount = dMesh->GetMeshNum();
	uint32 jointCount	 = dMesh->GetJointNum();

    AnimationClip CurrentAnimClip;
    if (dMesh->getAnimationClip(_currentAinmClipIndex, CurrentAnimClip))
    {
        for (int32 JointIndex = 0; JointIndex < CastValue<int32>(jointCount); ++JointIndex)
        {
            float RealFrame = CurrentAnimTime * 24.f;
            uint32 Frame = CastValue<uint32>(RealFrame);

            XMMATRIX JointMatrix = XMLoadFloat4x4(&CurrentAnimClip.GetKeyFrame(Frame).GetJointMatrix(JointIndex));

            // 다음 프레임과 블렌딩
            if (Frame < CurrentAnimClip.TotalFrame - 1)
            {
                float currentFrameFactor = 1.f - (RealFrame - CastValue<float>(Frame));
                float nextFrameFactor = 1.f - currentFrameFactor;
                JointMatrix = XMMatrixMultiply(JointMatrix, XMMatrixScaling(currentFrameFactor, currentFrameFactor, currentFrameFactor));

                XMMATRIX nextFrameMatrix = XMLoadFloat4x4(&CurrentAnimClip.GetKeyFrame(Frame + 1).GetJointMatrix(JointIndex));
                JointMatrix += XMMatrixMultiply(nextFrameMatrix, XMMatrixScaling(nextFrameFactor, nextFrameFactor, nextFrameFactor));
            }

            // 현재 프레임에서 조인트 행렬들
            XMMATRIX BindPoseInverseMatrix = XMLoadFloat4x4(&dMesh->GetJoint(JointIndex)._globalBindPoseInverseMatrix);
            XMStoreFloat4x4(&JointAnimMatrices[JointIndex], XMMatrixMultiply(BindPoseInverseMatrix, JointMatrix));
        }
    }
	
    PrimitiveDataList.reserve(geometryCount);
	for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
	{
		FPrimitiveData primitive = {};
		primitive.PrimitiveComponent = shared_from_this();
		primitive.PrimitiveType = EPrimitiveType::Mesh;
		primitive.MeshData = dMesh->GetMeshData(geometryIndex);
		primitive.Material = dMesh->getGeometryLinkMaterialIndex().size() > 0 ? dMesh->getMaterials()[dMesh->getGeometryLinkMaterialIndex()[geometryIndex]] : dMesh->getMaterials()[0];
        primitive._matrices = JointAnimMatrices;
        PrimitiveDataList.emplace_back(primitive);
	}

	if (dMesh->_pSkeleton)
	{
		//PrimitiveData primitive = {};
		//primitive._pPrimitive = shared_from_this();
		//primitive._pVertexBuffer = _pDynamicdMesh->_pSkeleton->getVertexBuffer();
		//primitive._pIndexBuffer = _pDynamicdMesh->_pSkeleton->getIndexBuffer();
		//primitive._pMaterial = _pDynamicdMesh->_pSkeleton->getMaterial();
		//primitive._primitiveType = EPrimitiveType::Mesh;

		//for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
		//{
		//	for (int32 jointIndex = 0; jointIndex < CastValue<int32>(jointCount); ++jointIndex)
		//	{

		//	}
		//}
		//primitive._matrices = _matrices;

		//primitiveDataList.emplace_back(primitive);
	}

    std::shared_ptr<MBoundingBox>& BoundingBox = dMesh->GetBoundingBox();
    if (BoundingBox && _bDrawBoundingBox)
    {
        FPrimitiveData PrimitiveData = {};
        PrimitiveData.PrimitiveComponent = shared_from_this();
        PrimitiveData.PrimitiveType = EPrimitiveType::Collision;
        PrimitiveData.MeshData = BoundingBox->GetMeshData();
        PrimitiveData.Material = BoundingBox->getMaterial();

        PrimitiveDataList.emplace_back(PrimitiveData);
    }

	return true;
}

void DynamicMeshComponent::SetMesh(const std::wstring& InPath)
{
    std::filesystem::path Path(InPath);

    if (Path.extension() == TEXT(".fbx"))
    {
        Mesh->LoadFromFBX(InPath);
    }
    else if (Path.extension() == TEXT(".json"))
    {
        Mesh->LoadFromAsset(InPath);
    }
}

uint32 DynamicMeshComponent::GetAnimClipNum()
{
    if (Mesh)
    {
        return GetSize(GetDynamicMesh()->GetAnimClips());
    }

    return 0;
}

std::shared_ptr<DynamicMesh> DynamicMeshComponent::GetDynamicMesh()
{
	return std::static_pointer_cast<DynamicMesh>(Mesh);
}

void DynamicMeshComponent::playAnimation(const uint32 index, const Time deltaTime)
{
	_currentAinmClipIndex = index;
	CurrentAnimTime += deltaTime;

    AnimationClip AnimClip;
    if (GetDynamicMesh()->getAnimationClip(index, AnimClip))
    {
        if (CurrentAnimTime > CastValue<float>(AnimClip.Duration))
        {
            CurrentAnimTime = 0.f;
        }
    }
}

/*
//bool bHasKeyFrames = !currentAnimClip._keyFrameLists[jointIndex][geometryIndex].empty();
//if (bHasKeyFrames == false)
//{
//
//    // 다른 메시에서 업데이트 된 경우
//    bool bUpdatedFromOtherMesh = matricesSet.find(jointIndex) != matricesSet.end();
//    if (bUpdatedFromOtherMesh)
//    {
//        continue;
//    }

//    // 본이 영향을 주는 버텍스가 없는 경우에는, 부모 본을 그대로 사용
//    int32 parentIndex = dMesh->getJoints()[jointIndex]._parentIndex;
//    if (parentIndex == -1)
//    {
//        _matrices[jointIndex] = IDENTITYMATRIX;
//    }
//    else
//    {
//        _matrices[jointIndex] = _matrices[parentIndex];
//    }
//
//}
//else
*/