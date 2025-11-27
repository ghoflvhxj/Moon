#include "StaticMeshComponent.h"

#include "Render.h"
#include "Renderer.h"
#include "GraphicDevice.h"
#include "Material.h"
#include "Texture.h"

#include "Core/ResourceManager.h"
#include "Module/Physics/Physics.h"
#include "Mesh/StaticMesh/StaticMesh.h"

#undef min
#undef max

using namespace DirectX;

StaticMeshComponent::StaticMeshComponent()
	: MMeshComponent()
{
    Mesh = std::make_shared<StaticMesh>();
}

StaticMeshComponent::StaticMeshComponent(const std::wstring& FilePath)
	: MMeshComponent()
{
	Mesh = std::make_shared<StaticMesh>();
    SetMesh(FilePath);
}

StaticMeshComponent::StaticMeshComponent(const std::wstring& FilePath, bool bUsePhysX, bool bUseRigidStatic)
{
	Mesh = std::make_shared<StaticMesh>();
    SetMesh(FilePath);
}

StaticMeshComponent::~StaticMeshComponent()
{

}

void StaticMeshComponent::Update(const Time deltaTime)
{
	if (PhysicsObject && bPhysicsSimulate)
	{
		setTranslation(PhysicsObject->GetPhysicsPos());
        setRotation(PhysicsObject->GetPhysicsRotation());

	}

	Super::Update(deltaTime);
}

const bool StaticMeshComponent::GetPrimitiveData(std::vector<FPrimitiveData> &PrimitiveDataList)
{
	if (nullptr == Mesh)
	{
		return false;
	}

    std::vector<std::shared_ptr<MMaterial>>& PrimitiveMaterials = Materials.empty() ? Mesh->getMaterials() : Materials;

	uint32 geometryCount = Mesh->GetMeshNum();
	PrimitiveDataList.reserve(geometryCount);

	for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
	{
        FPrimitiveData PrimitiveData = {};
		PrimitiveData.PrimitiveComponent = GetShared();
		PrimitiveData.PrimitiveType = EPrimitiveType::Mesh;
        PrimitiveData.MeshData = &Mesh->GetMeshData(geometryIndex);
		PrimitiveData.Material = Mesh->getGeometryLinkMaterialIndex().size() > 0 ? PrimitiveMaterials[Mesh->getGeometryLinkMaterialIndex()[geometryIndex]] : PrimitiveMaterials[0];

		PrimitiveDataList.push_back(PrimitiveData);
	}

	// BoudingBox
	//std::shared_ptr<MBoundingBox> &boundingBox = Mesh->GetBoundingBox();
	//if (boundingBox && _bDrawBoundingBox)
	//{
 //       FPrimitiveData PrimitiveData = {};
	//	PrimitiveData.PrimitiveComponent = GetShared();
	//	PrimitiveData.PrimitiveType = EPrimitiveType::Collision;
 //       PrimitiveData.MeshData = boundingBox->GetMeshData().get();
	//	PrimitiveData.Material = boundingBox->getMaterial();

	//	PrimitiveDataList.push_back(PrimitiveData);
	//}

    // 콜리젼
    if (PhysicsObject)
    {
        //PhysicsObject->GetMesh()
    }

	return true;
}

const bool StaticMeshComponent::GetBoundingBox(std::shared_ptr<MBoundingBox> &boundingBox)
{
	if (nullptr == Mesh)
	{
		return false;
	}

	boundingBox = Mesh->GetBoundingBox();
	return boundingBox != nullptr;
}

void StaticMeshComponent::setTranslation(const Vec3& translation)
{
    MSceneComponent::setTranslation(translation);

    if (PhysicsObject && bPhysicsSimulate == false)
    {
        PhysicsObject->SetPos(translation);
    }
}

void StaticMeshComponent::setScale(const Vec3& InScale)
{
	MSceneComponent::setScale(InScale);
    
    if (PhysicsObject && bPhysicsSimulate == false)
    {
        PhysicsObject->SetScale(InScale);
    }
}

void StaticMeshComponent::SetMesh(const std::wstring& InPath)
{
    std::filesystem::path Path = MFIleSystem::AbsolutePath(InPath);

    if (Path.extension() == TEXT(".fbx"))
    {
        Mesh->LoadFromFBX(Path);
    }
    else if (Path.extension() == TEXT(".json"))
    {
        g_ResourceManager->Load(InPath, Mesh);
    }

    Materials = Mesh->getMaterials();

    OnMeshChangedDelegate.Broadcast(this);
    OnPrimitiveChangedDelegate.Broadcast(this);
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