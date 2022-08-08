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

	// 바운딩 박스
	Vec3 min, max;
	fbxLoader.getBoundingBoxInfo(min, max);
	_pBoundingBox = std::make_shared<BoundingBox>(min, max);
}

const std::vector<uint32>& StaticMesh::getGeometryLinkMaterialIndex() const
{
	return _geometryLinkMaterialIndices;
}

MaterialList& StaticMesh::getMaterials()
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


const uint32 StaticMesh::getGeometryCount() const
{
	return _geometryCount;
}

std::shared_ptr<BoundingBox> StaticMesh::getBoudingBox()
{
	return _pBoundingBox;
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

const bool StaticMeshComponent::getPrimitiveData(std::vector<PrimitiveData> &primitiveDataList)
{
	if (nullptr == _pStaticMesh)
	{
		return false;
	}

	uint32 geometryCount = _pStaticMesh->getGeometryCount();
	primitiveDataList.reserve(geometryCount);

	for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
	{
		PrimitiveData primitive = {};
		primitive._pPrimitive = shared_from_this();
		primitive._pVertexBuffer = _pStaticMesh->getVertexBuffers()[geometryIndex];
		primitive._pIndexBuffer = _pStaticMesh->getIndexBuffer();
		primitive._pMaterial = _pStaticMesh->getMaterials()[_pStaticMesh->getGeometryLinkMaterialIndex()[geometryIndex]];
		primitive._primitiveType = EPrimitiveType::Mesh;
		primitive._pPrimitive = shared_from_this();
		primitiveDataList.emplace_back(primitive);
	}

	if (_bDrawBoundingBox)
	{
		// BoudingBox
		PrimitiveData primitive = {};
		primitive._pPrimitive = shared_from_this();
		primitive._pVertexBuffer = _pStaticMesh->getBoudingBox()->getVertexBuffer();
		primitive._pIndexBuffer = nullptr;
		primitive._pMaterial = _pStaticMesh->getBoudingBox()->getMaterial();;
		primitive._primitiveType = EPrimitiveType::Mesh;
		primitive._pPrimitive = shared_from_this();
		primitiveDataList.emplace_back(primitive);
	}

	return true;
}

std::shared_ptr<StaticMesh>& StaticMeshComponent::getStaticMesh()
{
	return _pStaticMesh;
}

void StaticMeshComponent::setDrawingBoundingBox(const bool bDraw)
{
	_bDrawBoundingBox = bDraw;
}

const bool StaticMeshComponent::IsDrawingBoundingBox() const
{
	return _bDrawBoundingBox;
}

BoundingBox::BoundingBox(const Vec3 &min, const Vec3 &max)
	: _min(min)
	, _max(max)
{
	// 정면
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });

	// 왼쪽면
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });

	// 아랫면
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });

	// 오른쪽면
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });

	// 윗면
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _min.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });

	// 뒷면
	_vertices.push_back({ Vec3{ _max.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _max.x, _min.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _max.y, _max.z } });
	_vertices.push_back({ Vec3{ _min.x, _min.y, _max.z } });

	_pVertexBuffer = std::make_shared<VertexBuffer>(CastValue<uint32>(sizeof(Vertex)), CastValue<uint32>(_vertices.size()), _vertices.data());

	_pMaterial = std::make_shared<Material>();
	_pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵
	_pMaterial->setFillMode(Graphic::FillMode::WireFrame);
}

std::shared_ptr<VertexBuffer> BoundingBox::getVertexBuffer()
{
	return _pVertexBuffer;
}

std::shared_ptr<Material> BoundingBox::getMaterial()
{
	return _pMaterial;
}
