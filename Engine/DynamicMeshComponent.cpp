#include "Include.h"
#include "DynamicMeshComponent.h"

#include "DynamicMeshComponentUtility.h"

#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

#include "Render.h"

#include "Texture.h"

#include "MainGame.h"
#include "Camera.h"

#include "FBXLoader.h"

using namespace DirectX;

class Skeleton
{
public:
	Skeleton(DynamicMesh* dynamicMesh);
public:
	std::vector<Vertex> _vertices;
	std::vector<Index>	_indices;

public:
	std::shared_ptr<MVertexBuffer> getVertexBuffer() { return _pVertexBuffer; }
	std::shared_ptr<MIndexBuffer> getIndexBuffer() { return nullptr; }
protected:
	std::shared_ptr<MVertexBuffer> _pVertexBuffer;
	std::shared_ptr<MIndexBuffer> _pIndexBuffer = nullptr;

public:
	std::shared_ptr<MMaterial> getMaterial() { return _pMaterial; }
protected:
	std::shared_ptr<MMaterial> _pMaterial;
};

Skeleton::Skeleton(DynamicMesh* dynamicMesh)
{
	for (uint32 i = 0; i < 199; ++i)
	{
		auto &trans = dynamicMesh->getJoints()[i]._position;
		_vertices.push_back({ Vec4{ trans.x, trans.y, trans.z, 1.f } });
		_vertices.back().BlendIndex[0] = i;
		_vertices.back().BlendWeight.x = 1.f;

		int32 parentIndex = dynamicMesh->getJoints()[i]._parentIndex;
		if (parentIndex != -1)
		{
			auto &parentTrans = dynamicMesh->getJoints()[parentIndex]._position;
			_vertices.push_back({ Vec4{ parentTrans.x, parentTrans.y, parentTrans.z, 1.f } });
			_vertices.back().BlendIndex[0] = parentIndex;
			_vertices.back().BlendWeight.x = 1.f;
		}
		else
		{
			_vertices.push_back({ Vec4{ trans.x, trans.y, trans.z, 1.f } });
		}
	}

	_pVertexBuffer = std::make_shared<MVertexBuffer>(CastValue<uint32>(sizeof(Vertex)), CastValue<uint32>(_vertices.size()), _vertices.data());
	_pMaterial = std::make_shared<MMaterial>();
	_pMaterial->setShader(TEXT("Bone.cso"), TEXT("TexPixelShader.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵
	_pMaterial->setTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
}

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

AnimationClip& DynamicMesh::getAnimationClip(const int index)
{
	return _animationClipList[index];
}

const uint32 DynamicMesh::getJointCount() const
{
	return _jointCount;
}

std::vector<FJoint>& DynamicMesh::getJoints()
{
	return _jointList;
}

DynamicMeshComponent::DynamicMeshComponent()
	: MPrimitiveComponent()
{

}

DynamicMeshComponent::DynamicMeshComponent(const std::wstring& FilePath)
	: MPrimitiveComponent()
{
	Mesh = std::make_shared<DynamicMesh>();
	Mesh->LoadFromFBX(FilePath);
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

	AnimationClip &currentAnimClip = Mesh->getAnimationClip(_currentAinmClipIndex);
	uint32 geometryCount = Mesh->getGeometryCount();
	uint32 jointCount	 = Mesh->getJointCount();

    PrimitiveDataList.reserve(geometryCount);

	std::set<uint32> matricesSet;
	for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
	{
		FPrimitiveData primitive = {};
		primitive.PrimitiveComponent = shared_from_this();
		primitive.Material = Mesh->getMaterials()[Mesh->getGeometryLinkMaterialIndex()[geometryIndex]];
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

std::shared_ptr<DynamicMesh>& DynamicMeshComponent::getDynamicMesh()
{
	return Mesh;
}

void DynamicMeshComponent::playAnimation(const uint32 index, const Time deltaTime)
{
	_currentAinmClipIndex = index;
	_currentPlayTime += deltaTime;

	if (_currentPlayTime > CastValue<float>(Mesh->getAnimationClip(index)._duration))
	{
		_currentPlayTime = 0.f;
	}
}