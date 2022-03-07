#include "stdafx.h"
#include "CollisionRenderer.h"

#include "Material.h"

#include "StaticMeshComponent.h"

CollisionRenderer::CollisionRenderer()
{
	initializeMeshInformation();
}

CollisionRenderer::~CollisionRenderer()
{

}

void CollisionRenderer::initializeMeshInformation()
{
	//_vertexList.reserve(8);
	//_vertexList.push_back({ Vec3{-0.5f, 0.5f, -0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	//_vertexList.push_back({ Vec3{0.5f, 0.5f, -0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	//_vertexList.push_back({ Vec3{0.5f, -0.5f, -0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	//_vertexList.push_back({ Vec3{-0.5f, -0.5f, -0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });

	//_vertexList.push_back({ Vec3{-0.5f, 0.5f, 0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	//_vertexList.push_back({ Vec3{0.5f, 0.5f, 0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	//_vertexList.push_back({ Vec3{0.5f, -0.5f, 0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });
	//_vertexList.push_back({ Vec3{-0.5f, -0.5f, 0.5f}, Vec4{1.f, 1.f, 1.f, 1.f} });

	//// x+
	//_indexList.reserve(36);
	//_indexList.push_back(1);
	//_indexList.push_back(5);
	//_indexList.push_back(6);

	//_indexList.push_back(1);
	//_indexList.push_back(6);
	//_indexList.push_back(2);

	//// x-
	//_indexList.push_back(4);
	//_indexList.push_back(0);
	//_indexList.push_back(3);

	//_indexList.push_back(4);
	//_indexList.push_back(3);
	//_indexList.push_back(7);

	//// y+
	//_indexList.push_back(4);
	//_indexList.push_back(5);
	//_indexList.push_back(1);

	//_indexList.push_back(4);
	//_indexList.push_back(1);
	//_indexList.push_back(0);

	//// y-
	//_indexList.push_back(3);
	//_indexList.push_back(2);
	//_indexList.push_back(6);

	//_indexList.push_back(3);
	//_indexList.push_back(6);
	//_indexList.push_back(7);

	//// z+
	//_indexList.push_back(7);
	//_indexList.push_back(6);
	//_indexList.push_back(5);

	//_indexList.push_back(7);
	//_indexList.push_back(5);
	//_indexList.push_back(4);

	//// z-
	//_indexList.push_back(0);
	//_indexList.push_back(1);
	//_indexList.push_back(2);

	//_indexList.push_back(0);
	//_indexList.push_back(2);
	//_indexList.push_back(3);

	//_pMaterial = std::make_shared<Material>(_vertexList, _indexList);
	 
}

void CollisionRenderer::render()
{
}

void CollisionRenderer::makeBox()
{
}
