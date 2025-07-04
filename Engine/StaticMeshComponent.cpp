#include "StaticMeshComponent.h"

#include "Render.h"
#include "Renderer.h"
#include "GraphicDevice.h"
#include "Material.h"
#include "Texture.h"

#include "Core/Physics/Physics.h"
#include "Mesh/StaticMesh/StaticMesh.h"

#include "MainGame.h"
#include "Camera.h"

#undef min
#undef max

using namespace DirectX;

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

}

void StaticMeshComponent::Update(const Time deltaTime)
{
	if (PhysicsObject && PhysicsObject->IsSimulating())
	{
		setTranslation(PhysicsObject->GetPhysicsPos());
	}

	MPrimitiveComponent::Update(deltaTime);
}

const bool StaticMeshComponent::GetPrimitiveData(std::vector<FPrimitiveData> &PrimitiveDataList)
{
	if (nullptr == _pStaticMesh)
	{
		return false;
	}

	uint32 geometryCount = _pStaticMesh->GetMeshNum();
	PrimitiveDataList.reserve(geometryCount);

	for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
	{
		FPrimitiveData PrimitiveData;
		PrimitiveData.PrimitiveComponent = shared_from_this();
		PrimitiveData.Material = _pStaticMesh->getGeometryLinkMaterialIndex().size() > 0 ? _pStaticMesh->getMaterials()[_pStaticMesh->getGeometryLinkMaterialIndex()[geometryIndex]] : _pStaticMesh->getMaterials()[0];
		PrimitiveData.PrimitiveType = EPrimitiveType::Mesh;
		PrimitiveData.MeshData = _pStaticMesh->GetMeshData(geometryIndex);

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

    // 콜리젼
    /*
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
    */

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

void StaticMeshComponent::setTranslation(const Vec3& translation)
{
    SceneComponent::setTranslation(translation);

    if (PhysicsObject && bPhysics)
    {
        PhysicsObject->SetPos(translation);
    }
}

void StaticMeshComponent::setScale(const Vec3& InScale)
{
	SceneComponent::setScale(InScale);
    
    if (PhysicsObject && bPhysics)
    {
        PhysicsObject->SetScale(InScale);
    }
}

Mat4& StaticMeshComponent::getWorldMatrix()
{
    // SoftBody 시뮬레이션의 경우 컴포넌트의 위치가 물리 시스템의 위치와 다르니 위치는 물리 시스템의 것을 사용해야 함. 
    //if (DeformableSurface)
    //{
    //    DeformalMatrix = SceneComponent::getWorldMatrix();
    //    DeformalMatrix.m[3][0] = 0.f;
    //    DeformalMatrix.m[3][1] = 0.f;
    //    DeformalMatrix.m[3][2] = 0.f;

    //    return DeformalMatrix;
    //}
    
    return SceneComponent::getWorldMatrix();
}

XMMATRIX StaticMeshComponent::GetRotationMatrix()
{
	if (PhysicsObject)
	{
		return XMMatrixRotationQuaternion(XMLoadFloat4(&PhysicsObject->GetPhysicsRotation()));
	}
	else
	{
		return SceneComponent::GetRotationMatrix();
	}
}

void StaticMeshComponent::SetMesh(const std::wstring& InPath)
{
    std::filesystem::path Path(InPath);
    if (Path.extension() == TEXT(".fbx"))
    {
        _pStaticMesh->LoadFromFBX(Path);
    }
    else if (Path.extension() == TEXT(".json"))
    {
        _pStaticMesh->LoadFromAsset(Path);
    }

    SetPhysics(bPhysics, true);
}

std::shared_ptr<StaticMesh>& StaticMeshComponent::getStaticMesh()
{
	return _pStaticMesh;
}

void StaticMeshComponent::Temp(float y)
{
    if (PhysicsObject)
    {
        PhysicsObject->AddForce(Vec3(0.f, y, 0.f));
    }
}

void StaticMeshComponent::SetGravity(bool bGravity)
{
    if (PhysicsObject)
    {
        PhysicsObject->SetGravity(bGravity);
    }
}

void StaticMeshComponent::Clothing()
{
    /*
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
        for (uint32 i=0; i<VertexNum; ++i)
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
        for (uint32 i = 0; i < VertexNum; ++i)
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
        AttachData.indices[0].count = GetSize(Indices);
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
        AttachData.coords[1].count = CastValue<PxU32>(Axis.size());
        AttachData.coords[1].stride = sizeof(PxVec4);

        PxDeformableAttachment* DeoformableAttachment = (*g_pPhysics)->createDeformableAttachment(AttachData);
        DeoformableAttachment->updatePose(PxTransform(PxIdentity));

        g_pPhysics->Scene->addActor(*DeformableSurface);
    }
    */

    if (g_pPhysics)
    {
        FPhysicsConstructData Data;
        Data.Mesh = _pStaticMesh;
        Data.PrimitiveComponent = shared_from_this();
        g_pPhysics->AddCloth(Data, PhysicsObject);
    }
}

void StaticMeshComponent::SetMass(float NewMass)
{
	if (PhysicsObject)
	{
        PhysicsObject->SetMass(NewMass);
	}
}

void StaticMeshComponent::SetAngularVelocity(float x, float y, float z)
{
    if (PhysicsObject)
    {
        PhysicsObject->SetAngularVelocity(Vec3(x, y, z));
    }
}

void StaticMeshComponent::SetVelocity(float x, float y, float z)
{
    if (PhysicsObject)
    {
        PhysicsObject->SetVelocity(Vec3(x, y, z));
    }
}

void StaticMeshComponent::SetPhysics(bool bInPhysics, bool bForce)
{
    if (bPhysics == bInPhysics && bForce == false)
    {
        return;
    }

    bPhysics = bInPhysics;

    if (g_pPhysics && bPhysics)
    {
        FPhysicsConstructData Data;
        Data.Mesh = _pStaticMesh;
        Data.PrimitiveComponent = shared_from_this();
        Data.PhysicsType = PhysicsType;
        if (g_pPhysics->AddPhysicsObject(Data, PhysicsObject))
        {
            //SetPhysics(bPhysics, true);
        }
    }
}

void StaticMeshComponent::SetPhysicsSimulate(bool bInSimulate, bool bForce /*= false*/)
{
    if (bPhysicsSimulate == bInSimulate && bForce == false)
    {
        return;
    }

    if (PhysicsObject == nullptr)
    {
        return;
    }

    bPhysicsSimulate = bInSimulate;

    if (bPhysics && PhysicsObject->IsSimulating() == false)
    {
        PhysicsObject->SetSimulate(true);
    }
    if (bPhysics == false && PhysicsObject->IsSimulating())
    {
        PhysicsObject->SetSimulate(false);
    }
}