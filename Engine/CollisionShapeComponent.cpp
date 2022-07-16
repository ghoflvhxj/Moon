#include "stdafx.h"
#include "CollisionShapeComponent.h"

#include "Material.h"

#include "Renderer.h"

CollisionShapeComponent::CollisionShapeComponent()
	: PrimitiveComponent()
{
	initialize();
}

CollisionShapeComponent::CollisionShapeComponent(std::vector<Vertex> &positionList)
{
	initialize(positionList);
}

CollisionShapeComponent::~CollisionShapeComponent()
{

}

void CollisionShapeComponent::initialize()
{
	_vertexList.reserve(8);
	_vertexList.push_back({ Vec3{-0.5f, 0.5f, -0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	_vertexList.push_back({ Vec3{0.5f, 0.5f, -0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	_vertexList.push_back({ Vec3{0.5f, -0.5f, -0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	_vertexList.push_back({ Vec3{-0.5f, -0.5f, -0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });

	_vertexList.push_back({ Vec3{-0.5f, 0.5f, 0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	_vertexList.push_back({ Vec3{0.5f, 0.5f, 0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	_vertexList.push_back({ Vec3{0.5f, -0.5f, 0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	_vertexList.push_back({ Vec3{-0.5f, -0.5f, 0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	
	// 앞
	_indexList.reserve(48);
	_indexList.push_back(0);
	_indexList.push_back(1);
	_indexList.push_back(1);
	_indexList.push_back(2);
	_indexList.push_back(2);
	_indexList.push_back(3);
	_indexList.push_back(3);
	_indexList.push_back(0);

	// 왼쪽
	_indexList.push_back(0);
	_indexList.push_back(3);
	_indexList.push_back(3);
	_indexList.push_back(7);
	_indexList.push_back(7);
	_indexList.push_back(4);
	_indexList.push_back(4);
	_indexList.push_back(0);

	// 오른쪽
	_indexList.push_back(1);
	_indexList.push_back(5);
	_indexList.push_back(5);
	_indexList.push_back(6);
	_indexList.push_back(6);
	_indexList.push_back(2);
	_indexList.push_back(2);
	_indexList.push_back(1);

	// 뒤
	_indexList.push_back(5);
	_indexList.push_back(6);
	_indexList.push_back(6);
	_indexList.push_back(7);
	_indexList.push_back(7);
	_indexList.push_back(4);
	_indexList.push_back(4);
	_indexList.push_back(5);

	// 위
	_indexList.push_back(0);
	_indexList.push_back(4);
	_indexList.push_back(4);
	_indexList.push_back(5);
	_indexList.push_back(5);
	_indexList.push_back(1);
	_indexList.push_back(1);
	_indexList.push_back(0);

	// 아래
	_indexList.push_back(3);
	_indexList.push_back(7);
	_indexList.push_back(7);
	_indexList.push_back(6);
	_indexList.push_back(6);
	_indexList.push_back(2);
	_indexList.push_back(2);
	_indexList.push_back(3);

	//_pMaterial = std::make_shared<Material>(_vertexList, _indexList);
	//_pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("LinePixelShader.cso"));
	//_pMaterial->setTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void CollisionShapeComponent::initialize(std::vector<Vertex> &positionList)
{
	_vertexList = positionList;

	uint32 vertexCount = static_cast<uint32>(_vertexList.size());
	for (uint32 indedx = 1; indedx < vertexCount; ++indedx)
	{
		_indexList.push_back(indedx - 1);
		_indexList.push_back(indedx);
	}

	//_pMaterial = std::make_shared<Material>(_vertexList, _indexList);
	//_pMaterial->setTopology(D3D_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
	//_pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("LinePixelShader.cso"));
}

void CollisionShapeComponent::Update(const Time deltaTime)
{
	SceneComponent::Update(deltaTime);

	//g_pRenderer->addCollisionShapeComponent(shared_from_this());
}

//void CollisionShapeComponent::render()
//{
//	_pMaterial->render(shared_from_this());
//}
