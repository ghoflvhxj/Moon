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

	// 모든 버텍스를 모아 하나의 버텍스만 사용... 굳이?
	//uint32 vertexCount = fbxLoader.getVertexCount();
	//std::vector<Vertex> allVertex;
	//allVertex.reserve(CastValue<size_t>(vertexCount));

	//for (auto &vertices : _verticesList)
	//{
	//	allVertex.assign(vertices.begin(), vertices.end());
	//}
	//_pVertexBuffer = std::make_shared<VertexBuffer>(CastValue<uint32>(sizeof(Vertex)), vertexCount, allVertex.data());

	// 인덱스 버퍼
	//uint32 indexCount = CastValue<uint32>(_indicesList[0].size());
	//_pIndexBuffer = std::make_shared<IndexBuffer>(sizeof(Index), indexCount, &_indicesList[0]);

	uint32 geometryCount = fbxLoader.getGeometryCount();
	_pVertexBuffers.reserve(CastValue<size_t>(geometryCount));
	for (uint32 i = 0; i < geometryCount; ++i)
	{
		_pVertexBuffers.emplace_back(std::make_shared<VertexBuffer>(CastValue<uint32>(sizeof(Vertex)), CastValue<uint32>(_verticesList[i].size()), _verticesList[i].data()));
	}
}

const std::vector<uint32>& StaticMesh::getGeometryLinkMaterialIndex() const
{
	return _geometryLinkMaterialIndices;
}

MaterialList StaticMesh::getMaterials() const
{
	return _materialList;
}

std::shared_ptr<Material> StaticMesh::getMaterial(const uint32 index)
{
	return _materialList[index];
}

const uint32 StaticMesh::getMaterialCount() const
{
	return CastValue<uint32>(_materialList.size());
}


std::vector<std::shared_ptr<VertexBuffer>> StaticMesh::getVertexBuffers()
{
	return _pVertexBuffers;
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
	if (nullptr == _pStaticMesh)
	{
		return false;
	}

	primitiveData._pVertexBuffers = _pStaticMesh->getVertexBuffers();
	primitiveData._pIndexBuffer = _pStaticMesh->getIndexBuffer();
	primitiveData._pMaterials = _pStaticMesh->getMaterials();
	primitiveData._pVertexShader = _pStaticMesh->getMaterial(0)->getVertexShader();
	primitiveData._pPixelShader = _pStaticMesh->getMaterial(0)->getPixelShader();
	primitiveData._geometryMaterialLinkIndex = _pStaticMesh->getGeometryLinkMaterialIndex();
	primitiveData._primitiveType = EPrimitiveType::Mesh;

	return true;
}

std::shared_ptr<StaticMesh>& StaticMeshComponent::getStaticMesh()
{
	return _pStaticMesh;
}