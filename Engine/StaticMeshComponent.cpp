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
    if (PhysXConvexMesh) PhysXConvexMesh->release();
    if (PhysxMaterial) PhysxMaterial->release();
}

void StaticMeshComponent::Update(const Time deltaTime)
{
	if (PhysicsObject)
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

	uint32 geometryCount = _pStaticMesh->getGeometryCount();
	PrimitiveDataList.reserve(geometryCount);

	for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
	{
		FPrimitiveData PrimitiveData;
		PrimitiveData.PrimitiveComponent = shared_from_this();
		PrimitiveData.Material = _pStaticMesh->getMaterials()[_pStaticMesh->getGeometryLinkMaterialIndex()[geometryIndex]];
		PrimitiveData.PrimitiveType = EPrimitiveType::Mesh;
		PrimitiveData.MeshData = _pStaticMesh->GetMeshData(geometryIndex);
		
        //if (bSimpleLayout)
        //{
        //    PrimitiveData.InputLayout = g_pGraphicDevice->SimpleLayout;
        //}

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

    if (PhysicsObject)
    {
        PhysicsObject->SetPos(translation);
    }
}

void StaticMeshComponent::setScale(const Vec3& InScale)
{
	SceneComponent::setScale(InScale);
    
    if (PhysicsObject)
    {
        PhysicsObject->SetScale(InScale);
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
	if (PhysicsObject)
	{
		return XMMatrixRotationQuaternion(XMLoadFloat4(&PhysicsObject->GetPhysicsRotation()));
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
        if (g_pPhysics->AddPhysicsObject(_pStaticMesh, PhysicsType, PhysicsObject))
        {
            SetPhysics(bPhysics, true);
        }
    }
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
        AttachData.coords[1].count = CastValue<PxU32>(Axis.size());
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

    if (bPhysics)
    {
        if (bAddedToScene == false)
        {
            PhysicsObject->SetPhysics(true);
            bAddedToScene = true;
        }
    }
    else
    {
        if (bAddedToScene)
        {
            PhysicsObject->SetPhysics(false);
            bAddedToScene = false;
        }
    }
}