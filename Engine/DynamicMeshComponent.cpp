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
		pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵

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

	uint32 geometryCount = _pDynamicMesh->getGeometryCount();
	uint32 jointCount	 = _pDynamicMesh->getJointCount();

	primitiveDataList.reserve(geometryCount);

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
			AnimationClip &currentAnimClip = _pDynamicMesh->getAnimationClip(_currentAinmClip);
			if (currentAnimClip._keyFrameLists[jointIndex][geometryIndex].empty())
			{
				continue;
			}

			float realFrame = _currentPlayTime * 24.f;
			uint32 frame = CastValue<uint32>(realFrame);

			float currentFrameFactor = 1.f - (realFrame - CastValue<float>(frame));
			float nextFrameFactor = 1.f - currentFrameFactor;

			XMMATRIX frameMatrix = XMLoadFloat4x4(&currentAnimClip._keyFrameLists[jointIndex][geometryIndex][frame]);
			if (frame < currentAnimClip._frameCount)
			{
				frameMatrix = XMMatrixMultiply(frameMatrix, XMMatrixScaling(currentFrameFactor, currentFrameFactor, currentFrameFactor));

				XMMATRIX nextFrameMatrix = XMLoadFloat4x4(&currentAnimClip._keyFrameLists[jointIndex][geometryIndex][frame + 1]);
				nextFrameMatrix = XMMatrixMultiply(nextFrameMatrix, XMMatrixScaling(nextFrameFactor, nextFrameFactor, nextFrameFactor));

				frameMatrix += nextFrameMatrix;
			}

			XMMATRIX inverseOfGlobalBindPoseMatrix = XMLoadFloat4x4(&_pDynamicMesh->getJoints()[jointIndex]._inverseOfGlobalBindPoseMatrices[geometryIndex]);
			XMStoreFloat4x4(&primitive._matrices[jointIndex], XMMatrixMultiply(inverseOfGlobalBindPoseMatrix, frameMatrix));
		}

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
	_currentAinmClip = index;
	_currentPlayTime += deltaTime;

	if (_currentPlayTime > CastValue<float>(_pDynamicMesh->getAnimationClip(index)._duration))
	{
		_currentPlayTime = 0.f;
	}
}