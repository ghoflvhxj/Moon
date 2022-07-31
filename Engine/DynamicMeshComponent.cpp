#include "stdafx.h"
#include "DynamicMeshComponent.h"

#include "DynamicMeshComponentUtility.h"

#include "GraphicDevice.h"
#include "Material.h"
#include "ConstantBuffer.h"

#include "Render.h"

#include "TextureComponent.h"

#include "MainGame.h"
#include "Camera.h"

#include "FBXLoader.h"

using namespace DirectX;

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

const bool DynamicMeshComponent::getPrimitiveData(PrimitiveData &primitiveData)
{
	if (nullptr == _pDynamicMesh)
	{
		return false;
	}

	primitiveData._pVertexBuffers = _pDynamicMesh->getVertexBuffers();
	primitiveData._pIndexBuffer = _pDynamicMesh->getIndexBuffer();
	primitiveData._pMaterials = _pDynamicMesh->getMaterials();
	primitiveData._pVertexShader = _pDynamicMesh->getMaterial(0)->getVertexShader();
	primitiveData._pPixelShader = _pDynamicMesh->getMaterial(0)->getPixelShader();
	primitiveData._geometryMaterialLinkIndex = _pDynamicMesh->getGeometryLinkMaterialIndex();
	primitiveData._primitiveType = EPrimitiveType::Mesh;

	return true;
}

std::shared_ptr<DynamicMesh>& DynamicMeshComponent::getDynamicMesh()
{
	return _pDynamicMesh;
}

void DynamicMeshComponent::playAnimation(const uint32 index)
{
	//XMMATRIX parentJointMatrix;
	//XMMATRIX currentjointMatrix;

	//uint32 keyFrameListCount = sizeToUint32(_animationClipList[0]._keyFrameLists.size());
	//for (int listIndex = 0; listIndex < keyFrameListCount; ++listIndex)
	//{
	//	int time = 0;
	//	_animationClipList[index]._keyFrameLists[listIndex][time]._matrix // listIndex가 joint의 인덱스
	//}
}

//void DynamicMeshComponent::render()
//{
//	static int matrixIndex = 0;
//	uint32 materialCount = sizeToUint32(_materialList.size());
//	uint32 keyFrameCount = sizeToUint32(_animationClipList[0]._keyFrameLists.size());
//	for (uint32 i = 0; i < materialCount; ++i)
//	{
//		// 애니메이션 행렬 업데이트
//		//if (true == _animationClipList[0]._keyFrameLists[0].empty())
//		//{
//		//	continue;
//		//}
//		// 
//		//auto pConstantBuffer = _materialList[i]->getVertexConstantBuffer(Material::VertexConstantBufferSlot::JointMatrix);
//		//if (nullptr != pConstantBuffer)
//		//{
//		//	ConstantBufferStruct::VertexStruct::JointMatrices jointMatrices;
//		//	for (uint32 j = 0; j < keyFrameCount; ++j)
//		//	{
//		//		//jointMatrices.jointMatrixList[j] = _animationClipList[0]._keyFrameLists[j][0]._matrix;
//		//		jointMatrices.jointMatrixList[j] = _animationClipList[0]._keyFrameLists[j][matrixIndex]._matrix;
//		//	}
//
//		//	pConstantBuffer->update(&jointMatrices, sizeof(jointMatrices));
//		//}
//		_materialList[i]->render(shared_from_this());
//	}
//}
