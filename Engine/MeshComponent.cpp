#include "MeshComponent.h"

#include "Mesh/StaticMesh/StaticMesh.h"
#include "Core/Physics/Physics.h"

MMeshComponent::MMeshComponent()
	: MPrimitiveComponent()
{
	//initializeMeshInformation();
}

MMeshComponent::~MMeshComponent()
{
}

void MMeshComponent::SetMesh(const std::wstring& InPath)
{
    std::filesystem::path Path(InPath);
    if (Path.extension() == TEXT(".fbx"))
    {
        Mesh->LoadFromFBX(Path);
    }
    else if (Path.extension() == TEXT(".json"))
    {
        Mesh->LoadFromAsset(Path);
    }

    SetPhysics(bPhysics, true);
    SetPhysicsSimulate(bPhysicsSimulate);
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
    if (g_pPhysics)
    {
        FPhysicsConstructData Data;
        Data.Mesh = Mesh;
        Data.PrimitiveComponent = shared_from_this();
        Data.PhysicsType = EPhysicsType::Dynamic;
        //g_pPhysics->AddCloth(Data, PhysicsObject);

        std::vector<FTest> t;
        {
            FTest a;
            a.MeshIndex = 7;
            a.InvMass.resize(Mesh->GetMeshData(7)->Vertices.size(), 1.f);
            t.push_back(a);
        }
        {
            FTest a;
            a.MeshIndex = 8;
            a.InvMass.resize(Mesh->GetMeshData(8)->Vertices.size(), 1.f);
            t.push_back(a);
        }
        {
            FTest a;
            a.MeshIndex = 11;
            a.InvMass.resize(Mesh->GetMeshData(11)->Vertices.size(), 1.f);
            t.push_back(a);
        }
        {
            FTest a;
            a.MeshIndex = 12;
            a.InvMass.resize(Mesh->GetMeshData(12)->Vertices.size(), 1.f);
            t.push_back(a);
        }
        {
            FTest a;
            a.MeshIndex = 13;
            a.InvMass.resize(Mesh->GetMeshData(13)->Vertices.size(), 1.f);
            t.push_back(a);
        }
        g_pPhysics->AddCloth(Data, t, PhysicsObject);


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
        Data.PrimitiveComponent = shared_from_this();
        Data.PhysicsType = PhysicsType;
        g_pPhysics->AddPhysicsObject(Data, PhysicsObject);

        //g_pPhysics->AddCloth();
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