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

#include "MPhysX.h"
#include <PxPhysicsAPI.h>

#include <DirectXMath.h>
using namespace DirectX;

#pragma region StaticMesh
void StaticMesh::initializeMeshInformation(const char *filePathName)
{
	FBXLoader fbxLoader(filePathName);

	VertexCount = fbxLoader.getVertexCount();
	_verticesList = std::move(fbxLoader.getVerticesList());
	_indicesList = std::move(fbxLoader.getIndicesList());
	_textureList = std::move(fbxLoader.getTextures());
	_geometryCount = fbxLoader.getGeometryCount();
	_geometryLinkMaterialIndices = fbxLoader.getLinkList();
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


	// 중심점과 모든 버텍스 위치
	uint32 VertexCount = fbxLoader.getVertexCount();
	AllVertexPosition.reserve(VertexCount);
	CenterPos = { 0.f, 0.f, 0.f };
	for (auto& vertices : _verticesList)
	{
		for (auto& vertex : vertices)
		{
			AllVertexPosition.emplace_back(Vec3(vertex.Pos.x, vertex.Pos.y, vertex.Pos.z));
			CenterPos.x += vertex.Pos.x;
			CenterPos.y += vertex.Pos.y;
			CenterPos.z += vertex.Pos.z;
		}
	}
	CenterPos.x /= CastValue<float>(VertexCount);
	CenterPos.y /= CastValue<float>(VertexCount);
	CenterPos.z /= CastValue<float>(VertexCount);
}

const std::vector<uint32>& StaticMesh::getGeometryLinkMaterialIndex() const
{
	return _geometryLinkMaterialIndices;
}

const std::vector<Vec3>& StaticMesh::GetAllVertexPosition() const
{
	return AllVertexPosition;
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

std::shared_ptr<BoundingBox> StaticMesh::getBoundingBox()
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

const Vec3& StaticMesh::GetCenterPos() const
{
	return CenterPos;
}

#pragma endregion

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

StaticMeshComponent::StaticMeshComponent(const char *filePathName, bool bUsePhysX, bool bUseRigidStatic)
{
	_pStaticMesh = std::make_shared<StaticMesh>();
	_pStaticMesh->initializeMeshInformation(filePathName);

	if (bUsePhysX)
	{
		PxMaterial *PhysxMaterial = (*g_pPhysics)->createMaterial(0.5f, 0.5f, 0.f);
		g_pPhysics->CreateConvex(_pStaticMesh->GetAllVertexPosition(), &PhysXConvexMesh);
		PxConvexMeshGeometry Geometry = PxConvexMeshGeometry(PhysXConvexMesh);
		PhysXShape = (*g_pPhysics)->createShape(Geometry, *PhysxMaterial);
		PhysXRigidStatic = (*g_pPhysics)->createRigidStatic(PxTransform(PxVec3(0.f, 0.f, 0.f)));
		PhysXRigidDynamic = (*g_pPhysics)->createRigidDynamic(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxIdentity)));

		if (PhysXRigidDynamic)
		{
			PhysXRigidDynamic->setMass(Mass);
			PhysXRigidDynamic->setAngularDamping(0.8f);
		}

		GetPhysXRigidActor()->attachShape(*PhysXShape);

		SetStaticCollision(bStaticCollision, true);
	}

	bUseRigidStatic = bUseRigidStatic;
}

StaticMeshComponent::~StaticMeshComponent()
{
}

void StaticMeshComponent::Update(const Time deltaTime)
{
	if (PxRigidActor* PhysXActor = GetPhysXRigidActor())
	{
		PxTransform PhysXTransform = PhysXActor->getGlobalPose();
		setTranslation(Vec3{ PhysXTransform.p.x, PhysXTransform.p.y, PhysXTransform.p.z });
	}

	PrimitiveComponent::Update(deltaTime);
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

	// BoudingBox
	std::shared_ptr<BoundingBox> &boundingBox = _pStaticMesh->getBoundingBox();
	if (boundingBox && _bDrawBoundingBox)
	{
		PrimitiveData primitive = {};
		primitive._pPrimitive = shared_from_this();
		primitive._pVertexBuffer = boundingBox->getVertexBuffer();
		primitive._pIndexBuffer = nullptr;
		primitive._pMaterial = boundingBox->getMaterial();
		primitive._primitiveType = EPrimitiveType::Collision;
		primitive._pPrimitive = shared_from_this();
		primitiveDataList.emplace_back(primitive);
	}

	return true;
}

const bool StaticMeshComponent::getBoundingBox(std::shared_ptr<BoundingBox> &boundingBox)
{
	if (nullptr == _pStaticMesh)
	{
		return false;
	}

	boundingBox = _pStaticMesh->getBoundingBox();
	return boundingBox != nullptr;
}

void StaticMeshComponent::setTranslation(const Vec3 &translation)
{
	SceneComponent::setTranslation(translation);

	if (PxRigidActor* RigidActor = GetPhysXRigidActor())
	{
		PxTransform PhysXTransform = RigidActor->getGlobalPose();
		PhysXTransform.p.x = translation.x;
		PhysXTransform.p.y = translation.y;
		PhysXTransform.p.z = translation.z;

		if (PxRigidStatic* PhysXRigidStatic = GetPhysXRigidStatic())
		{
			PhysXRigidDynamic->setGlobalPose(PhysXTransform);
		}

		if (PxRigidStatic* PhysXRigidDynamic = GetPhysXRigidStatic())
		{
			PhysXRigidDynamic->setGlobalPose(PhysXTransform);
		}
	}
}

void StaticMeshComponent::setScale(const Vec3 &scale)
{
	SceneComponent::setScale(scale);

	if (_pStaticMesh)
	{
		if (PxRigidDynamic* PhysXRigidDynamic = GetPhysXRigidDynamic())
		{
			Vec3 CenterPos = _pStaticMesh->GetCenterPos();
			CenterPos.x *= scale.x;
			CenterPos.y *= scale.y;
			CenterPos.z *= scale.z;
			PhysXRigidDynamic->setCMassLocalPose(PxTransform(CenterPos.x, CenterPos.y, CenterPos.z));
		}

		PxRigidActor* PhysXRigidActor = GetPhysXRigidActor();

		if (PxShape* PhysXShape = GetPhysXShape())
		{
			PhysXRigidActor->detachShape(*PhysXShape);

			PxGeometryHolder holder = PhysXShape->getGeometry();
			holder.convexMesh().convexMesh->acquireReference();

			holder.convexMesh().scale.scale = PxVec3(scale.x, scale.y, scale.z);
			PhysXShape->setGeometry(holder.any());

			PhysXRigidActor->attachShape(*PhysXShape);

			holder.convexMesh().convexMesh->release();
		}
	}
}

XMMATRIX StaticMeshComponent::GetRotationMatrix()
{
	if (PxRigidActor* PhysXActor = GetPhysXRigidActor())
	{
		PxTransform PhysXTransform = PhysXActor->getGlobalPose();
		XMFLOAT4 Quaternion = { PhysXTransform.q.x, PhysXTransform.q.y, PhysXTransform.q.z, PhysXTransform.q.w };
		return XMMatrixRotationQuaternion(XMLoadFloat4(&Quaternion));
	}
	else
	{
		return SceneComponent::GetRotationMatrix();
	}
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


void StaticMeshComponent::Temp(float y)
{
	if (PxRigidDynamic* PhysXRigidDynamic = GetPhysXRigidDynamic())
	{
		PhysXRigidDynamic->addForce(PxVec3(0.f, y, 0.f));
		PhysXRigidDynamic->addTorque(PxVec3(y * 10.f, 0.f, 0.f));
	}
}

void StaticMeshComponent::SetGravity(bool bGravity)
{
	if (PxRigidDynamic* PhysxRigidDynamic = GetPhysXRigidDynamic())
	{
		PhysxRigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, bGravity);
		PhysxRigidDynamic->setAngularVelocity(PxVec3(0.f, 0.f, 0.f));
		PhysxRigidDynamic->wakeUp();
	}
}

void StaticMeshComponent::SetStaticCollision(bool bNewStaticCollision, bool bForce)
{
	if (g_pPhysics == nullptr)
	{
		return;
	}

	if (bNewStaticCollision == bStaticCollision && bForce == false)
	{
		return;
	}

	PxRigidStatic* PhysXRigidStatic = GetPhysXRigidStatic();
	PxRigidDynamic* PhysXRigidDynamic = GetPhysXRigidDynamic();

	if (PhysXRigidStatic && PhysXRigidDynamic)
	{
		GetPhysXRigidActor()->detachShape(*PhysXShape);

		if (bNewStaticCollision == true)
		{
			g_pPhysics->Scene->removeActor(*PhysXRigidDynamic);
			g_pPhysics->Scene->addActor(*PhysXRigidStatic);
		}
		else
		{
			g_pPhysics->Scene->removeActor(*PhysXRigidStatic);
			g_pPhysics->Scene->addActor(*PhysXRigidDynamic);
		}
		//else
		//{
		//	g_pPhysics->Scene->removeActor(*PhysXRigidStatic);
		//	g_pPhysics->Scene->removeActor(*PhysXRigidDynamic);
		//}

		bStaticCollision = bNewStaticCollision;

		GetPhysXRigidActor()->attachShape(*PhysXShape);
	}
}

void StaticMeshComponent::SetMass(float NewMass)
{
	if (PxRigidDynamic* PhysXRigidDynamic = GetPhysXRigidDynamic())
	{
		PhysXRigidDynamic->setMass(Mass);
	}
}

physx::PxRigidActor* StaticMeshComponent::GetPhysXRigidActor()
{
	return (bStaticCollision == true) ? static_cast<PxRigidActor*>(PhysXRigidStatic) : static_cast<PxRigidActor*>(PhysXRigidDynamic);;
}

physx::PxRigidDynamic* StaticMeshComponent::GetPhysXRigidDynamic()
{
	return PhysXRigidDynamic;
}

physx::PxRigidStatic* StaticMeshComponent::GetPhysXRigidStatic()
{
	return PhysXRigidStatic;
}

physx::PxShape* StaticMeshComponent::GetPhysXShape()
{
	return PhysXShape;
}