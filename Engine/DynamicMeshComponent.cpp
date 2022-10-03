#include "stdafx.h"
#include "DynamicMeshComponent.h"

#include "DynamicMeshComponentUtility.h"

#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

#include "Render.h"

#include "TextureComponent.h"

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
	std::shared_ptr<VertexBuffer> getVertexBuffer() { return _pVertexBuffer; }
	std::shared_ptr<IndexBuffer> getIndexBuffer() { return nullptr; }
protected:
	std::shared_ptr<VertexBuffer> _pVertexBuffer;
	std::shared_ptr<IndexBuffer> _pIndexBuffer = nullptr;

public:
	std::shared_ptr<Material> getMaterial() { return _pMaterial; }
protected:
	std::shared_ptr<Material> _pMaterial;
};

Skeleton::Skeleton(DynamicMesh* dynamicMesh)
{
	for (uint32 i = 0; i < 199; ++i)
	{
		auto &trans = dynamicMesh->getJoints()[i]._position;
		_vertices.push_back({ Vec3{ trans.x, trans.y, trans.z } });
		_vertices.back().BlendIndex[0] = i;
		_vertices.back().BlendWeight.x = 1.f;

		int32 parentIndex = dynamicMesh->getJoints()[i]._parentIndex;
		if (parentIndex != -1)
		{
			auto &parentTrans = dynamicMesh->getJoints()[parentIndex]._position;
			_vertices.push_back({ Vec3{ parentTrans.x, parentTrans.y, parentTrans.z } });
			_vertices.back().BlendIndex[0] = parentIndex;
			_vertices.back().BlendWeight.x = 1.f;
		}
		else
		{
			_vertices.push_back({ Vec3{ trans.x, trans.y, trans.z } });
		}
	}

	_pVertexBuffer = std::make_shared<VertexBuffer>(CastValue<uint32>(sizeof(Vertex)), CastValue<uint32>(_vertices.size()), _vertices.data());
	_pMaterial = std::make_shared<Material>();
	_pMaterial->setShader(TEXT("Bone.cso"), TEXT("TexPixelShader.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵
	_pMaterial->setTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
}

void DynamicMesh::initializeMeshInformation(const char *filePathName)
{
	FBXLoader fbxLoader(filePathName, _animationClipList);

	_verticesList = std::move(fbxLoader.getVerticesList());
	_indicesList = std::move(fbxLoader.getIndicesList());
	_textureList = std::move(fbxLoader.getTextures());
	_geometryCount = fbxLoader.getGeometryCount();
	_geometryLinkMaterialIndices = std::move(fbxLoader.getLinkList());
	if (_geometryLinkMaterialIndices.size() == 0)
	{
		_geometryLinkMaterialIndices.emplace_back(0);
	}

	uint32 materialCount = fbxLoader.getMaterialCount();
	uint32 fixedMaterialCount = (materialCount > 0) ? materialCount : 1;
	_materialList.reserve(CastValue<size_t>(fixedMaterialCount));
	for (uint32 i = 0; i < fixedMaterialCount; ++i)
	{
		std::shared_ptr<Material> pMaterial = std::make_shared<Material>();

		bool applyedFixedMaterial = (materialCount == 0);
		if (false == applyedFixedMaterial)
		{
			pMaterial->setTexture(_textureList[i]);
		}
		pMaterial->setShader(TEXT("TexAnimVertexShader.cso"), TEXT("TexPixelShader.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵

		_materialList.push_back(pMaterial);
	}

	_jointList = std::move(fbxLoader._jointList);
	_jointCount = CastValue<uint32>(_jointList.size());

	uint32 geometryCount = fbxLoader.getGeometryCount();
	_pVertexBuffers.reserve(CastValue<size_t>(geometryCount));
	for (uint32 i = 0; i < geometryCount; ++i)
	{
		_pVertexBuffers.emplace_back(std::make_shared<VertexBuffer>(CastValue<uint32>(sizeof(Vertex)), CastValue<uint32>(_verticesList[i].size()), _verticesList[i].data()));
	}

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
	: PrimitiveComponent()
{

}

DynamicMeshComponent::DynamicMeshComponent(const char *filePathName)
	: PrimitiveComponent()
{
	_pDynamicMesh = std::make_shared<DynamicMesh>();
	_pDynamicMesh->initializeMeshInformation(filePathName);
}

DynamicMeshComponent::~DynamicMeshComponent()
{
}

const bool DynamicMeshComponent::getPrimitiveData(std::vector<PrimitiveData> &primitiveDataList)
{
	if (nullptr == _pDynamicMesh)
	{
		return false;
	}

	AnimationClip &currentAnimClip = _pDynamicMesh->getAnimationClip(_currentAinmClipIndex);
	uint32 geometryCount = _pDynamicMesh->getGeometryCount();
	uint32 jointCount	 = _pDynamicMesh->getJointCount();

	primitiveDataList.reserve(geometryCount);

	std::set<uint32> matricesSet;
	for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
	{
		PrimitiveData primitive = {};
		primitive._pPrimitive = shared_from_this();
		primitive._pVertexBuffer = _pDynamicMesh->getVertexBuffers()[geometryIndex];
		primitive._pIndexBuffer = _pDynamicMesh->getIndexBuffer();
		primitive._pMaterial = _pDynamicMesh->getMaterials()[_pDynamicMesh->getGeometryLinkMaterialIndex()[geometryIndex]];
		primitive._primitiveType = EPrimitiveType::Mesh;

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
				int32 parentIndex = _pDynamicMesh->getJoints()[jointIndex]._parentIndex;
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

				XMMATRIX globalBindPoseInverseMatrix = XMLoadFloat4x4(&_pDynamicMesh->getJoints()[jointIndex]._globalBindPoseInverseMatrix);
				XMStoreFloat4x4(&_matrices[jointIndex], XMMatrixMultiply(globalBindPoseInverseMatrix, frameMatrix));

				matricesSet.insert(jointIndex);
			}
		}

		primitive._matrices = _matrices;

		primitiveDataList.emplace_back(primitive);
	}

	if (_pDynamicMesh->_pSkeleton)
	{
		PrimitiveData primitive = {};
		primitive._pPrimitive = shared_from_this();
		primitive._pVertexBuffer = _pDynamicMesh->_pSkeleton->getVertexBuffer();
		primitive._pIndexBuffer = _pDynamicMesh->_pSkeleton->getIndexBuffer();
		primitive._pMaterial = _pDynamicMesh->_pSkeleton->getMaterial();
		primitive._primitiveType = EPrimitiveType::Mesh;

		for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
		{
			for (int32 jointIndex = 0; jointIndex < CastValue<int32>(jointCount); ++jointIndex)
			{

			}
		}
		primitive._matrices = _matrices;

		primitiveDataList.emplace_back(primitive);
	}

	return true;
}

std::shared_ptr<DynamicMesh>& DynamicMeshComponent::getDynamicMesh()
{
	return _pDynamicMesh;
}

void DynamicMeshComponent::playAnimation(const uint32 index, const Time deltaTime)
{
	_currentAinmClipIndex = index;
	_currentPlayTime += deltaTime;

	if (_currentPlayTime > CastValue<float>(_pDynamicMesh->getAnimationClip(index)._duration))
	{
		_currentPlayTime = 0.f;
	}
}