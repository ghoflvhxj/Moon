#include "stdafx.h"
#include "StaticMeshComponent.h"

#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

#include "Render.h"

#include "TextureComponent.h"

#include "MainGame.h"
#include "Camera.h"

#include "FBXLoader.h"

void StaticMesh::initializeMeshInformation(const char *filePathName)
{
	FBXLoader fbxLoader(filePathName);

	_verticesList = std::move(fbxLoader.getVerticesList());
	_indicesList = std::move(fbxLoader.getIndicesList());
	_textureList = std::move(fbxLoader.getTextures());

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
		pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader2.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵

		_materialList.push_back(pMaterial);
	}

	uint32 vertexCount = fbxLoader.getVertexCount();
	std::vector<Vertex> allVertex;
	allVertex.reserve(CastValue<size_t>(vertexCount));

	for (auto &vertices : _verticesList)
	{
		allVertex.assign(vertices.begin(), vertices.end());
	}
	_pVertexBuffer = std::make_shared<VertexBuffer>(CastValue<uint32>(sizeof(Vertex)), vertexCount, allVertex.data());

	//uint32 indexCount = CastValue<uint32>(_indicesList[0].size());
	//_pIndexBuffer = std::make_shared<IndexBuffer>(sizeof(Index), indexCount, &_indicesList[0]);
}

std::shared_ptr<Material> StaticMesh::getMaterial(const uint32 index)
{
	return _materialList[index];
}

const uint32 StaticMesh::getMaterialCount() const
{
	return CastValue<uint32>(_materialList.size());
}


std::shared_ptr<VertexBuffer> StaticMesh::getVertexBuffer()
{
	return _pVertexBuffer;
}

std::shared_ptr<IndexBuffer> StaticMesh::getIndexBuffer()
{
	return _pIndexBuffer;
}

StaticMeshComponent::StaticMeshComponent()
	: PrimitiveComponent()
{

}

StaticMeshComponent::StaticMeshComponent(const char *filePathName)
	: PrimitiveComponent()
{
	_pStaticMesh = std::make_shared<StaticMesh>();
	_pStaticMesh->initializeMeshInformation(filePathName);
}

StaticMeshComponent::~StaticMeshComponent()
{
}

const bool StaticMeshComponent::getPrimitiveData(PrimitiveData &primitiveData)
{
	std::shared_ptr<StaticMesh> &pStaticMesh = getStaticMesh();
	if (nullptr == pStaticMesh)
	{
		return false;
	}

	primitiveData._pVertexBuffer = pStaticMesh->getVertexBuffer();
	primitiveData._pIndexBuffer = pStaticMesh->getIndexBuffer();
	primitiveData._pMaterial = pStaticMesh->getMaterial(0);
	primitiveData._pVertexShader = pStaticMesh->getMaterial(0)->getVertexShader();
	primitiveData._pPixelShader = pStaticMesh->getMaterial(0)->getPixelShader();
	return true;
}

std::shared_ptr<StaticMesh>& StaticMeshComponent::getStaticMesh()
{
	return _pStaticMesh;
}