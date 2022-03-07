#include "stdafx.h"
#include "StaticMeshComponent.h"

#include "GraphicDevice.h"
#include "Material.h"

#include "TextureComponent.h"

#include "MainGame.h"
#include "Camera.h"

#include "FBXLoader.h"

StaticMeshComponent::StaticMeshComponent()
	: PrimitiveComponent()
	, _vertexList()
	, _indexList()
	, _textureList()
{

}

StaticMeshComponent::StaticMeshComponent(const char *filePathName)
	: PrimitiveComponent()
	, _vertexList()
	, _indexList()
	, _textureList()
{
	initializeMeshInformation(filePathName);
}

StaticMeshComponent::~StaticMeshComponent()
{
}

void StaticMeshComponent::initializeMeshInformation(const char *filePathName)
{
	FBXLoader fbxLoader(filePathName);

	_vertexList		= std::move(fbxLoader.getVertices());
	_indexList		= std::move(fbxLoader.getIndices());
	_textureList	= std::move(fbxLoader.getTextures());

	uint32 materialCount = fbxLoader.getMaterialCount();
	_materialList.reserve(materialCount);
	for (uint32 i = 0; i < materialCount; ++i)
	{
		std::shared_ptr<Material> pMaterial = std::make_shared<Material>(_vertexList[i], _indexList[i]);
		pMaterial->setTexture(_textureList[i]);
		pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader2.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵

		_materialList.push_back(pMaterial);
	}
}

void StaticMeshComponent::render()
{
	uint32 materialCount = static_cast<int32>(_materialList.size());
	for (uint32 i = 0; i < materialCount; ++i)
	{
		_materialList[i]->render(shared_from_this());
	}
}

std::shared_ptr<Material> StaticMeshComponent::getMaterial(const uint32 index)
{
	return _materialList[index];
}
