#include "Include.h"
#include "StaticMeshComponent.h"

#include "GraphicDevice.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

#include "Render.h"
#include "Renderer.h"

#include "Texture.h"

#include "MainGame.h"
#include "Camera.h"

#include "FBXLoader.h"

#include "MPhysX.h"
#include "NvidiaPhysX/PxPhysicsAPI.h"


#include <DirectXMath.h>
using namespace DirectX;
using namespace physx;

#undef min
#undef max

void StaticMesh::LoadFromFBX(const std::wstring& FilePath)
{
	MFBXLoader FbxLoader;
	InitializeFromFBX(FbxLoader, FilePath);

	const std::vector<VertexList>& Vertices = FbxLoader.getVerticesList();
	const std::vector<IndexList>& Indices = FbxLoader.getIndicesList();

	// FBX를 이용해 Serialize할 데이터들을 저장
	GeometryNum = FbxLoader.GetGeometryNum();
	for (uint32 GeometryIndex = 0; GeometryIndex < GeometryNum; ++GeometryIndex)
	{
		auto MeshData = std::make_shared<FMeshData>();
		MeshData->Vertices = Vertices[GeometryIndex];
		MeshData->Indices = Indices[GeometryIndex];
		MeshDataList.push_back(MeshData);
	}

	// 중심점과 모든 버텍스 위치
    TotalVertexNum = FbxLoader.GetTotalVertexNum();
	AllVertexPosition.reserve(TotalVertexNum);
	CenterPos = { 0.f, 0.f, 0.f };
	for (auto& vertices : Vertices)
	{
		for (auto& vertex : vertices)
		{
			AllVertexPosition.emplace_back(Vec3(vertex.Pos.x, vertex.Pos.y, vertex.Pos.z));
			CenterPos.x += vertex.Pos.x;
			CenterPos.y += vertex.Pos.y;
			CenterPos.z += vertex.Pos.z;
		}
	}
	CenterPos.x /= CastValue<float>(TotalVertexNum);
	CenterPos.y /= CastValue<float>(TotalVertexNum);
	CenterPos.z /= CastValue<float>(TotalVertexNum);
}

void StaticMesh::InitializeFromFBX(MFBXLoader& FbxLoader, const std::wstring& FilePath)
{
	FbxLoader.LoadMesh(FilePath);

	Textures = FbxLoader.GetTextures(); // MaterialTextures
	_geometryLinkMaterialIndices = FbxLoader.getLinkList();
	if (_geometryLinkMaterialIndices.size() == 0)
	{
		_geometryLinkMaterialIndices.emplace_back(0);
	}

	uint32 MaterialNum = std::max(1u, FbxLoader.GetMaterialNum());
	Materials.reserve(MaterialNum);
	for (uint32 MaterialIndex = 0; MaterialIndex < MaterialNum; ++MaterialIndex)
	{
		std::shared_ptr<MMaterial>& NewMaterial = std::make_shared<MMaterial>();

		if (MaterialIndex < Textures.size())
		{
			NewMaterial->setTextures(Textures[MaterialIndex]);
		}
		
		NewMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso"));
		Materials.push_back(NewMaterial);
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

	// 바운딩 박스
    Vec3 Min, Max;
    FbxLoader.getBoundingBoxInfo(Min, Max);
    _pBoundingBox = std::make_shared<MBoundingBox>(Min, Max);

    // 바운딩 스피어
    float Radius = XMVectorGetX(XMVector3Length(XMLoadFloat3(&Min) - XMLoadFloat3(&Max)));

}

const std::vector<uint32>& StaticMesh::getGeometryLinkMaterialIndex() const
{
	return _geometryLinkMaterialIndices;
}

const std::vector<Vec3>& StaticMesh::GetAllVertexPosition() const
{
	return AllVertexPosition;
}

const std::shared_ptr<FMeshData>& StaticMesh::GetMeshData(const uint32 Index) const
{
	if (Index < GetMeshNum())
	{
		return MeshDataList[Index];
	}
	
	return nullptr;
}

MaterialList& StaticMesh::getMaterials()
{
	return Materials;
}

std::shared_ptr<MMaterial> StaticMesh::getMaterial(const uint32 index)
{
	return Materials[index];
}

const uint32 StaticMesh::GetMaterialNum() const
{
	return CastValue<uint32>(Materials.size());
}


const uint32 StaticMesh::getGeometryCount() const
{
	return GeometryNum;
}

std::shared_ptr<MBoundingBox> StaticMesh::GetBoundingBox()
{
	return _pBoundingBox;
}

std::vector<std::shared_ptr<MVertexBuffer>> StaticMesh::getVertexBuffers()
{
	return _pVertexBuffers;
}

std::shared_ptr<MIndexBuffer> StaticMesh::getIndexBuffer()
{
	return _pIndexBuffer;
}

const Vec3& StaticMesh::GetCenterPos() const
{
	return CenterPos;
}

StaticMeshComponent::StaticMeshComponent()
	: MPrimitiveComponent()
{
    _pStaticMesh = std::make_shared<StaticMesh>();
}

StaticMeshComponent::StaticMeshComponent(const std::wstring& FilePath)
	: MPrimitiveComponent()
{
	_pStaticMesh = std::make_shared<StaticMesh>();
    SetMesh(FilePath);
}

StaticMeshComponent::StaticMeshComponent(const std::wstring& FilePath, bool bUsePhysX, bool bUseRigidStatic)
{
	_pStaticMesh = std::make_shared<StaticMesh>();
    SetMesh(FilePath);
}

StaticMeshComponent::~StaticMeshComponent()
{
    if (PhysXRigidDynamic) PhysXRigidDynamic->release();
    if (PhysXRigidStatic) PhysXRigidStatic->release();
    if (PhysXShape) PhysXShape->release();
    if (PhysXConvexMesh) PhysXConvexMesh->release();
    if (PhysxMaterial) PhysxMaterial->release();
}

void StaticMeshComponent::Update(const Time deltaTime)
{
	if (PxRigidActor* PhysXActor = GetPhysXRigidActor())
	{
		PxTransform PhysXTransform = PhysXActor->getGlobalPose();
		setTranslation(Vec3{ PhysXTransform.p.x, PhysXTransform.p.y, PhysXTransform.p.z });
	}

	MPrimitiveComponent::Update(deltaTime);
}

const bool StaticMeshComponent::GetPrimitiveData(std::vector<FPrimitiveData> &PrimitiveDataList)
{
	if (nullptr == _pStaticMesh)
	{
		return false;
	}

	uint32 geometryCount = _pStaticMesh->getGeometryCount();
	PrimitiveDataList.reserve(geometryCount);

	for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
	{
		FPrimitiveData PrimitiveData;
		PrimitiveData.PrimitiveComponent = shared_from_this();
		PrimitiveData.Material = _pStaticMesh->getMaterials()[_pStaticMesh->getGeometryLinkMaterialIndex()[geometryIndex]];
		PrimitiveData.PrimitiveType = EPrimitiveType::Mesh;
		PrimitiveData.MeshData = _pStaticMesh->GetMeshData(geometryIndex);
		
        if (bSimpleLayout)
        {
            PrimitiveData.InputLayout = g_pGraphicDevice->SimpleLayout;
        }

		PrimitiveDataList.push_back(PrimitiveData);
	}

	// BoudingBox
	std::shared_ptr<MBoundingBox> &boundingBox = _pStaticMesh->GetBoundingBox();
	if (boundingBox && _bDrawBoundingBox)
	{
		FPrimitiveData PrimitiveData = {};
		PrimitiveData.PrimitiveComponent = shared_from_this();
		PrimitiveData.Material = boundingBox->getMaterial();
		PrimitiveData.PrimitiveType = EPrimitiveType::Collision;
        PrimitiveData.MeshData = boundingBox->GetMeshData();

		PrimitiveDataList.push_back(PrimitiveData);
	}

    // 피직스
    if (GetPhysXRigidActor() && (bDrawColliision || g_pRenderer->bDrawCollision))
    {
        PxGeometryHolder GeometryHolder(GetPhysXShape()->getGeometry());
        PxConvexMeshGeometry Geometry = GeometryHolder.convexMesh();
           
        if (MaterialForPhysX == nullptr)
        {
            MaterialForPhysX = std::make_shared<MMaterial>();
            MaterialForPhysX->setShader(TEXT("VertexShader.cso"), TEXT("PixelShader.cso"));
            MaterialForPhysX->setFillMode(Graphic::FillMode::WireFrame);
            MaterialForPhysX->setCullMode(Graphic::CullMode::None);
        }

        if (MeshDataForPhysX == nullptr)
        {
            MeshDataForPhysX = std::make_shared<FMeshData>();

            for (int i = 0; i < Geometry.convexMesh->getNbVertices(); ++i)
            {
                const PxVec3& Position = Geometry.convexMesh->getVertices()[i];
                Vertex Vtx;
                Vtx.Pos.x = Position.x;
                Vtx.Pos.y = Position.y;
                Vtx.Pos.z = Position.z;
                MeshDataForPhysX->Vertices.push_back(Vtx);
                
            }
        }

        FPrimitiveData PrimitiveData = {};
        PrimitiveData.PrimitiveComponent = shared_from_this();
        PrimitiveData.Material = MaterialForPhysX;
        PrimitiveData.PrimitiveType = EPrimitiveType::Collision;
        PrimitiveData.MeshData = MeshDataForPhysX;

        PrimitiveDataList.push_back(PrimitiveData);
    }

	return true;
}

const bool StaticMeshComponent::GetBoundingBox(std::shared_ptr<MBoundingBox> &boundingBox)
{
	if (nullptr == _pStaticMesh)
	{
		return false;
	}

	boundingBox = _pStaticMesh->GetBoundingBox();
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
        RigidActor->setGlobalPose(PhysXTransform);
	}
}

void StaticMeshComponent::setScale(const Vec3 &scale)
{
	SceneComponent::setScale(scale);

	if (_pStaticMesh)
	{
        if (PhysXRigidDynamic)
        {
            Vec3 CenterPos = _pStaticMesh->GetCenterPos();
            CenterPos.x *= scale.x;
            CenterPos.y *= scale.y;
            CenterPos.z *= scale.z;

            PhysXRigidDynamic->setCMassLocalPose(PxTransform(CenterPos.x, CenterPos.y, CenterPos.z));
        }

		if (PxShape* PhysXShape = GetPhysXShape())
		{
		    PxRigidActor* PhysXRigidActor = GetPhysXRigidActor();
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

Mat4& StaticMeshComponent::getWorldMatrix()
{
    if (DeformableSurface)
    {
        DeformalMatrix = SceneComponent::getWorldMatrix();
        DeformalMatrix.m[3][0] = 0.f;
        DeformalMatrix.m[3][1] = 0.f;
        DeformalMatrix.m[3][2] = 0.f;

        return DeformalMatrix;
    }
    
    return SceneComponent::getWorldMatrix();
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

void StaticMeshComponent::SetMesh(const std::wstring& Path)
{
    _pStaticMesh->LoadFromFBX(Path);

    if (g_pPhysics && bPhysics)
    {
        PhysxMaterial = (*g_pPhysics)->createMaterial(0.5f, 0.5f, 0.f);
        g_pPhysics->CreateConvex(_pStaticMesh->GetAllVertexPosition(), &PhysXConvexMesh);
        PxConvexMeshGeometry Geometry = PxConvexMeshGeometry(PhysXConvexMesh);
        PhysXShape = (*g_pPhysics)->createShape(Geometry, *PhysxMaterial, true);

        switch (PhysicsType)
        {
        case EPhysicsType::Static:
            PhysXRigidStatic = (*g_pPhysics)->createRigidStatic(PxTransform(PxVec3(0.f, 0.f, 0.f)));
            break;
        case EPhysicsType::Dynamic:
            PhysXRigidDynamic = (*g_pPhysics)->createRigidDynamic(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxIdentity)));
            PhysXRigidDynamic->setMass(Mass);
            PhysXRigidDynamic->setAngularDamping(1.f);
            break;
        }

        GetPhysXRigidActor()->attachShape(*PhysXShape);
        SetPhysics(bPhysics, true);
    }
}

std::shared_ptr<StaticMesh>& StaticMeshComponent::getStaticMesh()
{
	return _pStaticMesh;
}

void StaticMeshComponent::Temp(float y)
{
	if (PxRigidDynamic* PhysXRigidDynamic = GetPhysXRigidDynamic())
	{
		PhysXRigidDynamic->addForce(PxVec3(0.f, y, 0.f));
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

void StaticMeshComponent::Clothing()
{
    // Desc
    PxTriangleMeshDesc TriangleMeshDesc;
    std::vector<Vec4> Vertices;
    for (Vec3 temp : _pStaticMesh->GetAllVertexPosition())
    {
        Vertices.push_back({ temp.x, temp.y, temp.z, 1.f });
    }

    TriangleMeshDesc.points.data = (void*)Vertices.data();
    TriangleMeshDesc.points.stride = sizeof(PxVec4);
    TriangleMeshDesc.points.count = static_cast<PxU32>(Vertices.size());

    TriangleMeshDesc.triangles.data = (void*)_pStaticMesh->GetMeshData(0)->Indices.data();
    TriangleMeshDesc.triangles.stride = 3 * sizeof(PxU32);
    TriangleMeshDesc.triangles.count = static_cast<PxU32>(_pStaticMesh->GetMeshData(0)->Indices.size() / 3);

    PxTolerancesScale ToleranceScale(0.1f, 10.f);
    PxCookingParams CookingParams(ToleranceScale);
    CookingParams.meshPreprocessParams.raise(PxMeshPreprocessingFlag::eFORCE_32BIT_INDICES);
    CookingParams.meshPreprocessParams.clear(PxMeshPreprocessingFlag::eENABLE_VERT_MAPPING);
    CookingParams.buildGPUData = true;

    PxDefaultMemoryOutputStream OutStream;
    if (PxCookTriangleMesh(CookingParams, TriangleMeshDesc, OutStream))
    {
        PxDefaultMemoryInputData inStream(OutStream.getData(), OutStream.getSize());
        PxTriangleMesh* TriangleMesh = (*g_pPhysics)->createTriangleMesh(inStream);

        PxTriangleMeshGeometry TriangleMeshGeometry(TriangleMesh);
        PxDeformableSurfaceMaterial* TriangleMeshMat = (*g_pPhysics)->createDeformableSurfaceMaterial(1000000.f, 0.1f, 0.1f);
        PxShape* TriangleMeshShape = (*g_pPhysics)->createShape(TriangleMeshGeometry, *TriangleMeshMat, true);

        PxTransform Tf;
        Tf.p.x = getWorldTranslation().x;
        Tf.p.x = getWorldTranslation().y;
        Tf.p.x = getWorldTranslation().z;

        DeformableSurface = (*g_pPhysics)->createDeformableSurface(*g_pPhysics->Scene->getCudaContextManager());
        DeformableSurface->attachShape(*TriangleMeshShape);
        DeformableSurface->setDeformableSurfaceFlag(PxDeformableSurfaceFlag::eUSE_ANISOTROPIC_MODEL, true);
        VertexNum = TriangleMesh->getNbVertices();

        // Restpos
        CUdeviceptr devBuf = reinterpret_cast<CUdeviceptr>(DeformableSurface->getRestPositionBufferD());
        std::vector<PxVec4> initBuf(VertexNum);
        for (int i=0; i<VertexNum; ++i)
        {
            Vec4& t = Vertices[i];
            initBuf[i].x = t.x;
            initBuf[i].y = t.y;
            initBuf[i].z = t.z;
            initBuf[i].x += getWorldTranslation().x;
            initBuf[i].y += getWorldTranslation().y;
            initBuf[i].z += getWorldTranslation().z;
            initBuf[i].w = 1.f;
        }

        cuMemcpyHtoD(devBuf, initBuf.data(), sizeof(PxVec4) * VertexNum);
        DeformableSurface->markDirty(PxDeformableSurfaceDataFlag::eREST_POSITION);

        //PosInvmass
        devBuf = reinterpret_cast<CUdeviceptr>(DeformableSurface->getPositionInvMassBufferD());
        for (int i = 0; i < VertexNum; ++i)
        {
            Vec4& t = Vertices[i];
            initBuf[i].x = t.x;
            initBuf[i].y = t.y;
            initBuf[i].z = t.z;
            initBuf[i].x += getWorldTranslation().x;
            initBuf[i].y += getWorldTranslation().y;
            initBuf[i].z += getWorldTranslation().z;

            initBuf[i].w = t.y > 0.2f || t.y < -0.2f ? 0.f : 10.f;
            initBuf[i].w = t.y > 0.f ? 0.f : 1000.f;
            initBuf[i].w = 1000.f;
        }
        cuMemcpyHtoD(devBuf, initBuf.data(), sizeof(PxVec4) * VertexNum);
        DeformableSurface->markDirty(PxDeformableSurfaceDataFlag::ePOSITION_INVMASS);
    }

    if (DeformableSurface)
    {
        std::vector<uint32> Indices;
        for (int i = 0; i < Vertices.size(); ++i)
        {
            if (Vertices[i].z > 0.2f)
            {
                Indices.push_back(i);
            }
        }

        // 액터 유형 -> 타입 유형 -> 데이터 결정
        PxDeformableAttachmentData AttachData;
        AttachData.actor[0] = DeformableSurface;
        AttachData.type[0] = PxDeformableAttachmentTargetType::eVERTEX;
        AttachData.indices[0].data = Indices.data();
        AttachData.indices[0].count = Indices.size();
        AttachData.indices[0].stride = sizeof(uint32);

        AttachData.actor[1] = nullptr;
        AttachData.type[1] = PxDeformableAttachmentTargetType::eWORLD;
        AttachData.pose[1] = PxTransform(PxIdentity);
        std::vector<PxVec4> Axis;
        for (uint32 Index : Indices)
        {
            PxVec4 AttachWorldPos = PxVec4(getWorldTranslation().x + Vertices[Index].x, getWorldTranslation().y + Vertices[Index].y, getWorldTranslation().z + Vertices[Index].z, 1.f);
            Axis.push_back(AttachWorldPos);
        }
        AttachData.coords[1].data = Axis.data();
        AttachData.coords[1].count = Axis.size();
        AttachData.coords[1].stride = sizeof(PxVec4);

        PxDeformableAttachment* DeoformableAttachment = (*g_pPhysics)->createDeformableAttachment(AttachData);
        DeoformableAttachment->updatePose(PxTransform(PxIdentity));

        g_pPhysics->Scene->addActor(*DeformableSurface);
    }
}

void StaticMeshComponent::UpdateClothing()
{
    if (DeformableSurface)
    {
        if (auto VB = g_pRenderer->GetVertexBuffer(PrimitiveID))
        {
            VB->UpdateUsingCUDA(DeformableSurface, VertexNum);
        }
    }
}

void StaticMeshComponent::SetMass(float NewMass)
{
	if (PxRigidDynamic* PhysXRigidDynamic = GetPhysXRigidDynamic())
	{
		PhysXRigidDynamic->setMass(Mass);
	}
}

void StaticMeshComponent::SetAngularVelocity(float x, float y, float z)
{
    if (PxRigidDynamic* PhysXRigidDynamic = GetPhysXRigidDynamic())
    {
        PhysXRigidDynamic->setAngularVelocity(PxVec3(x, y, z));
    }
}

void StaticMeshComponent::SetVelocity(float x, float y, float z)
{
    if (PxRigidDynamic* PhysXRigidDynamic = GetPhysXRigidDynamic())
    {
        PhysXRigidDynamic->setLinearVelocity(PxVec3(x, y, z));
    }
}

void StaticMeshComponent::SetPhysics(bool bInPhysics, bool bForce)
{
    if (bPhysics == bInPhysics && bForce == false)
    {
        return;
    }

    bPhysics = bInPhysics;

    if (bPhysics)
    {
        if (bAddedToScene == false)
        {
            g_pPhysics->Scene->addActor(*GetPhysXRigidActor());
            bAddedToScene = true;
        }
    }
    else
    {
        if (bAddedToScene)
        {
            g_pPhysics->Scene->removeActor(*GetPhysXRigidActor());
            bAddedToScene = false;
        }
    }
}

physx::PxRigidActor* StaticMeshComponent::GetPhysXRigidActor()
{
    if (PhysXRigidStatic)
    {
        return PhysXRigidStatic;
    }

    return PhysXRigidDynamic;
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