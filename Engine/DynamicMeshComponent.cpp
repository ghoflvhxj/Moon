#include "DynamicMeshComponent.h"
#include "DynamicMeshComponentUtility.h"

#include "Render.h"
#include "GraphicDevice.h"
#include "Material.h"
#include "Texture.h"

#include "Core/DynamicMesh/DynamicMesh.h"

#include "MainGame.h"
#include "Camera.h"

using namespace DirectX;

DynamicMeshComponent::DynamicMeshComponent()
	: MPrimitiveComponent()
{
    Mesh = std::make_shared<DynamicMesh>();
}

DynamicMeshComponent::DynamicMeshComponent(const std::wstring& FilePath)
	: MPrimitiveComponent()
{
	Mesh = std::make_shared<DynamicMesh>();
    SetMesh(FilePath);
}

DynamicMeshComponent::~DynamicMeshComponent()
{
}

void DynamicMeshComponent::Update(const Time deltaTime)
{
    MPrimitiveComponent::Update(deltaTime);

    if (IsAnimPlaying())
    {
        playAnimation(0, deltaTime);
    }
}

const bool DynamicMeshComponent::GetPrimitiveData(std::vector<FPrimitiveData> & PrimitiveDataList)
{
	if (nullptr == Mesh)
	{
		return false;
	}

    AnimationClip currentAnimClip;
    Mesh->getAnimationClip(_currentAinmClipIndex, currentAnimClip);
	uint32 geometryCount = Mesh->GetMeshNum();
	uint32 jointCount	 = Mesh->getJointCount();

    PrimitiveDataList.reserve(geometryCount);

	std::set<uint32> matricesSet;
	for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
	{
		FPrimitiveData primitive = {};
		primitive.PrimitiveComponent = shared_from_this();
		primitive.Material = Mesh->getGeometryLinkMaterialIndex().size() > 0 ? Mesh->getMaterials()[Mesh->getGeometryLinkMaterialIndex()[geometryIndex]] : Mesh->getMaterials()[0];
		primitive.PrimitiveType = EPrimitiveType::Mesh;
		primitive.MeshData = Mesh->GetMeshData(geometryIndex);

		for (int32 jointIndex = 0; jointIndex < CastValue<int32>(jointCount); ++jointIndex)
		{
			bool bHasKeyFrames = !currentAnimClip._keyFrameLists[jointIndex][geometryIndex].empty();
			if (bHasKeyFrames == false)
			{
				// 다른 메시에서 업데이트 된 경우
				bool bUpdatedFromOtherMesh = matricesSet.find(jointIndex) != matricesSet.end();
				if (bUpdatedFromOtherMesh)  
				{
					continue;
				}

				// 본이 영향을 주는 버텍스가 없는 경우에는, 부모 본을 그대로 사용
				int32 parentIndex = Mesh->getJoints()[jointIndex]._parentIndex;
				if (parentIndex == -1)
				{
					_matrices[jointIndex] = IDENTITYMATRIX;
				}
				else
				{
					_matrices[jointIndex] = _matrices[parentIndex];
				}
			}
			else
			{
				float realFrame = _currentPlayTime * 30.f; // 하드코딩 삭제하기
				uint32 frame = CastValue<uint32>(realFrame);

				XMMATRIX frameMatrix = XMLoadFloat4x4(&currentAnimClip._keyFrameLists[jointIndex][geometryIndex][frame]);

				// 다음 프레임과 블렌딩
				if (frame < currentAnimClip._frameCount - 1)
				{
					float currentFrameFactor = 1.f - (realFrame - CastValue<float>(frame));
					float nextFrameFactor = 1.f - currentFrameFactor;

					frameMatrix = XMMatrixMultiply(frameMatrix, XMMatrixScaling(currentFrameFactor, currentFrameFactor, currentFrameFactor));

					XMMATRIX nextFrameMatrix = XMLoadFloat4x4(&currentAnimClip._keyFrameLists[jointIndex][geometryIndex][frame + 1]);
					nextFrameMatrix = XMMatrixMultiply(nextFrameMatrix, XMMatrixScaling(nextFrameFactor, nextFrameFactor, nextFrameFactor));

					frameMatrix += nextFrameMatrix;
				}

				XMMATRIX globalBindPoseInverseMatrix = XMLoadFloat4x4(&Mesh->getJoints()[jointIndex]._globalBindPoseInverseMatrix);
				XMStoreFloat4x4(&_matrices[jointIndex], XMMatrixMultiply(globalBindPoseInverseMatrix, frameMatrix));

				matricesSet.insert(jointIndex);
			}
		}

		primitive._matrices = _matrices;

        PrimitiveDataList.emplace_back(primitive);
	}

	if (Mesh->_pSkeleton)
	{
		//PrimitiveData primitive = {};
		//primitive._pPrimitive = shared_from_this();
		//primitive._pVertexBuffer = _pDynamicMesh->_pSkeleton->getVertexBuffer();
		//primitive._pIndexBuffer = _pDynamicMesh->_pSkeleton->getIndexBuffer();
		//primitive._pMaterial = _pDynamicMesh->_pSkeleton->getMaterial();
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

    std::shared_ptr<MBoundingBox>& BoundingBox = Mesh->GetBoundingBox();
    if (BoundingBox && _bDrawBoundingBox)
    {
        FPrimitiveData PrimitiveData = {};
        PrimitiveData.PrimitiveComponent = shared_from_this();
        PrimitiveData.Material = BoundingBox->getMaterial();
        PrimitiveData.PrimitiveType = EPrimitiveType::Collision;
        PrimitiveData.MeshData = BoundingBox->GetMeshData();

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

std::shared_ptr<DynamicMesh>& DynamicMeshComponent::getDynamicMesh()
{
	return Mesh;
}

void DynamicMeshComponent::playAnimation(const uint32 index, const Time deltaTime)
{
	_currentAinmClipIndex = index;
	_currentPlayTime += deltaTime;

    AnimationClip AnimClip;
    if (Mesh->getAnimationClip(index, AnimClip))
    {
        if (_currentPlayTime > CastValue<float>(AnimClip._duration))
        {
            _currentPlayTime = 0.f;
        }
    }
}