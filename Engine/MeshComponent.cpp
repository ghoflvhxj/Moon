#include "MeshComponent.h"

#include "Renderer.h"
#include "Core/Physics/Physics.h"
#include "Core/ResourceManager.h"

MMeshComponent::MMeshComponent()
	: MPrimitiveComponent()
{
	//initializeMeshInformation();
}

MMeshComponent::~MMeshComponent()
{
}

void MMeshComponent::SetMesh(const std::wstring& InPath, bool bSetPhyiscs)
{
    std::filesystem::path Path(InPath);

    if (Path.extension() == TEXT(".fbx"))
    {
        Mesh->LoadFromFBX(Path);
    }
    else if (Path.extension() == TEXT(".json"))
    {
        g_ResourceManager->Load(InPath, Mesh);
    }

    Materials = Mesh->getMaterials();
    bDirty = true;

    if (bPhysics && bSetPhyiscs)
    {
        SetPhysics(bPhysics, true);
        SetPhysicsSimulate(bPhysicsSimulate);
    }

    OnMeshChangedDelegate.Broadcast(GetShared());
    OnPrimitiveChangedDelegate.Broadcast(GetShared());
}

std::shared_ptr<StaticMesh> MMeshComponent::GetMesh()
{
    return Mesh;
}


void MMeshComponent::AddForce(const Vec3& InForce)
{
    if (PhysicsObject)
    {
        PhysicsObject->AddForce(InForce);
    }
}

void MMeshComponent::Clothing()
{
    
}

void MMeshComponent::RemovePhysics()
{
    if (PhysicsObject)
    {
        PhysicsObject->Remove();
        PhysicsObject.reset();
    }
}

void MMeshComponent::SetPhysics(bool bInPhysics, bool bForce)
{
    if (bPhysics == bInPhysics && bForce == false)
    {
        return;
    }

    bPhysics = bInPhysics;

    if (g_pPhysics && bPhysics)
    {
        FPhysicsConstructData Data;
        Data.Mesh = Mesh;
        Data.PrimitiveComponent = GetShared();
        Data.PhysicsType = PhysicsType;
        g_pPhysics->AddPhysicsObject(Data, PhysicsObject);
    }
}

void MMeshComponent::SetPhysicsSimulate(bool bInSimulate, bool bForce /*= false*/)
{
    if (PhysicsObject == nullptr)
    {
        return;
    }

    if (bInSimulate != PhysicsObject->IsSimulating())
    {
        bPhysicsSimulate = bInSimulate;
        PhysicsObject->SetSimulate(bPhysicsSimulate);
    }
}