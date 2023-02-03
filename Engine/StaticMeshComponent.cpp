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

void StaticMesh::initializeMeshInformation(const char *filePathName, bool bUsePhysX, bool bRigidStatic)
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
		pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso")); // ������ ������ ���̴��� �о�� �ϴµ�, ������ �����ϱ� �׳� �ӽ÷� ����

		_materialList.push_back(pMaterial);
	}

	// ��� ���ؽ��� ��� �ϳ��� ���ؽ��� ���... ����?
	//uint32 vertexCount = fbxLoader.getVertexCount();
	//std::vector<Vertex> allVertex;
	//allVertex.reserve(CastValue<size_t>(vertexCount));

	//for (auto &vertices : _verticesList)
	//{
	//	allVertex.assign(vertices.begin(), vertices.end());
	//}
	//_pVertexBuffer = std::make_shared<VertexBuffer>(CastValue<uint32>(sizeof(Vertex)), vertexCount, allVertex.data());

	// �ε��� ����
	//uint32 indexCount = CastValue<uint32>(_indicesList[0].size());
	//_pIndexBuffer = std::make_shared<IndexBuffer>(sizeof(Index), indexCount, &_indicesList[0]);

	uint32 geometryCount = fbxLoader.getGeometryCount();
	_pVertexBuffers.reserve(CastValue<size_t>(geometryCount));
	for (uint32 i = 0; i < geometryCount; ++i)
	{
		_pVertexBuffers.emplace_back(std::make_shared<VertexBuffer>(CastValue<uint32>(sizeof(Vertex)), CastValue<uint32>(_verticesList[i].size()), _verticesList[i].data()));
	}

	// �ٿ�� �ڽ�
	Vec3 min, max;
	fbxLoader.getBoundingBoxInfo(min, max);
	_pBoundingBox = std::make_shared<BoundingBox>(min, max);


	// ������ ConvexMesh
	if (bUsePhysX)
	{
		uint32 VertexCount = fbxLoader.getVertexCount();
		std::vector<Vec3> PxVertices;
		PxVertices.reserve(VertexCount);
		CenterPos = { 0.f, 0.f, 0.f };
		for (auto& vertices : _verticesList)
		{
			for (auto& vertex : vertices)
			{
				PxVertices.emplace_back(Vec3(vertex.Pos.x, vertex.Pos.y, vertex.Pos.z));
				CenterPos.x += vertex.Pos.x;
				CenterPos.y += vertex.Pos.y;
				CenterPos.z += vertex.Pos.z;
			}
		}
		CenterPos.x /= CastValue<float>(VertexCount);
		CenterPos.y /= CastValue<float>(VertexCount);
		CenterPos.z /= CastValue<float>(VertexCount);

		PxMaterial *PhysxMaterial = (*g_pPhysics)->createMaterial(0.5f, 0.5f, 0.f);
		g_pPhysics->CreateConvex(PxVertices, &PhysXConvexMesh);
		PxConvexMeshGeometry Geometry = PxConvexMeshGeometry(PhysXConvexMesh);
		PhysXShape = (*g_pPhysics)->createShape(Geometry, *PhysxMaterial);

		bStatic = bRigidStatic;
		if (bStatic)
		{
			PhysxRigidActor = (*g_pPhysics)->createRigidStatic(PxTransform(PxVec3(0.f, 0.f, 0.f)));
		}
		else
		{
			PxRigidDynamic* PhysxRigidDynamic = (*g_pPhysics)->createRigidDynamic(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxIdentity)));
			PhysxRigidDynamic->setMass(3.f);
			PhysxRigidDynamic->setCMassLocalPose(PxTransform(PxVec3(0.f, 0.2f, 0.f), PxQuat(PxIdentity)));
			//PhysxRigidDynamic->setMaxDepenetrationVelocity(0.002f);
			PhysxRigidDynamic->setAngularDamping(0.8f);
			PhysxRigidActor = PhysxRigidDynamic;
		}

		PhysxRigidActor->attachShape(*PhysXShape);
		g_pPhysics->Scene->addActor(*PhysxRigidActor); 
	}
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

void StaticMeshComponent::Temp(float y)
{
	if (PxRigidBody* PhysXRigidBody = _pStaticMesh->GetPhysXRigidDynamic())
	{
		PhysXRigidBody->addForce(PxVec3(0.f, y, 0.f));
		PhysXRigidBody->addTorque(PxVec3(y, 0.f, 0.f));
	}
}

void StaticMeshComponent::SetGravity(bool bGravity)
{
	if (PxRigidDynamic* PhysxRigidDynamic = _pStaticMesh->GetPhysXRigidDynamic())
	{
		PhysxRigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, bGravity);
		PhysxRigidDynamic->setAngularVelocity(PxVec3(0.f, 0.f, 0.f));
		PhysxRigidDynamic->wakeUp();
	}
}

physx::PxRigidActor* StaticMesh::GetPhysXActor()
{
	return PhysxRigidActor;
}

physx::PxRigidDynamic* StaticMesh::GetPhysXRigidDynamic()
{
	return (bStatic == true) ? nullptr : static_cast<PxRigidDynamic*>(PhysxRigidActor);
}

physx::PxRigidStatic* StaticMesh::GetPhysXRigidStatic()
{
	return (bStatic == true) ? static_cast<PxRigidStatic*>(PhysxRigidActor) : nullptr;
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

StaticMeshComponent::StaticMeshComponent(const char *filePathName, bool bUsePhysX, bool bRigidStatic)
{
	_pStaticMesh = std::make_shared<StaticMesh>();
	_pStaticMesh->initializeMeshInformation(filePathName, bUsePhysX, bRigidStatic);
}

StaticMeshComponent::~StaticMeshComponent()
{
}

void StaticMeshComponent::Update(const Time deltaTime)
{
	if (PxRigidActor* PhysXActor = _pStaticMesh->GetPhysXActor())
	{
		PxTransform PhysXTransform = PhysXActor->getGlobalPose();
		setTranslation(PhysXTransform.p.x, PhysXTransform.p.y, PhysXTransform.p.z);

		Vec3 Scale = getScale();
		if (PxRigidDynamic* PhysXRigidDynamic = _pStaticMesh->GetPhysXRigidDynamic())
		{
			Vec3 CenterPos = _pStaticMesh->CenterPos;
			CenterPos.x *= Scale.x;
			CenterPos.y *= Scale.y;
			CenterPos.z *= Scale.z;
			PhysXRigidDynamic->setCMassLocalPose(PxTransform(CenterPos.x, CenterPos.y, CenterPos.z));
		}


		PxShape* PhysXShape = nullptr;
		PhysXActor->getShapes(&PhysXShape, 1);
		if (PhysXShape)
		{
			PhysXActor->detachShape(*PhysXShape);

			PxGeometryHolder holder = PhysXShape->getGeometry();
			holder.convexMesh().convexMesh->acquireReference();


			
			holder.convexMesh().scale.scale = PxVec3(Scale.x, Scale.y, Scale.z);
			PhysXShape->setGeometry(holder.any());

			PhysXActor->attachShape(*PhysXShape);
			holder.convexMesh().convexMesh->release();
		}
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

XMMATRIX StaticMeshComponent::GetRotationMatrix()
{
	if (PxRigidActor* PhysXActor = _pStaticMesh->GetPhysXActor())
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