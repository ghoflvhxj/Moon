#include "stdafx.h"
#include "DynamicMeshComponent.h"

#include "DynamicMeshComponentUtility.h"

#include "GraphicDevice.h"
#include "Material.h"
#include "ConstantBuffer.h"

#include "TextureComponent.h"

#include "MainGame.h"
#include "Camera.h"

#include "FBXLoader.h"

using namespace DirectX;

DynamicMeshComponent::DynamicMeshComponent()
	: PrimitiveComponent()
	, _verticesList()
	, _indicesList()
	, _texturesList()
{

}

DynamicMeshComponent::DynamicMeshComponent(const char *filePathName)
	: PrimitiveComponent()
	, _verticesList()
	, _indicesList()
	, _texturesList()
{
	initializeMeshInformation(filePathName);
}

DynamicMeshComponent::~DynamicMeshComponent()
{
}

void DynamicMeshComponent::initializeMeshInformation(const char *filePathName)
{
	//FBXLoader fbxLoader(filePathName, _animationClipList);

	//_verticesList = std::move(fbxLoader.getVerticesList());
	//_indicesList = std::move(fbxLoader.getIndicesList());
	//_texturesList = std::move(fbxLoader.getTextures());

	//uint32 vertexCount = ToUint32(_verticesList.size());
	//_originVertexPositionList.reserve(vertexCount);
	//for (uint32 i = 0; i < vertexCount; ++i)
	//{
	//	_originVertexPositionList.push_back(_verticesList[0][i].Pos);
	//}

	//const std::vector<int> &linkList = fbxLoader.getLinkList();
	//const uint32 linkListCount = static_cast<uint32>(linkList.size());

	//uint32 materialCount = fbxLoader.getMaterialCount();
	//_materialList.reserve(materialCount);
	//for (uint32 materialIndex = 0; materialIndex < materialCount; ++materialIndex)
	//{
	//	std::vector<VertexList> verticesList;
	//	std::vector<IndexList> indicesList;
	//	for (uint32 linkIndex = 0; linkIndex < linkListCount; ++linkIndex)
	//	{
	//		if (linkList[linkIndex] != materialIndex)
	//			continue;

	//		verticesList.push_back(_verticesList[linkIndex]);
	//		indicesList.push_back(_indicesList[linkIndex]);
	//	}

	//	std::shared_ptr<Material> pMaterial = std::make_shared<Material>(verticesList, indicesList);
	//	pMaterial->setTexture(_texturesList[materialIndex]);
	//	pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader2.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵

	//	_materialList.push_back(pMaterial);
	//}
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

std::shared_ptr<Material> DynamicMeshComponent::getMaterial(const uint32 index)
{
	return _materialList[index];
}
