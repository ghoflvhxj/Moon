#include "Physics.h"

void MPhysicsEngine::StartSimulate()
{
    std::cout << "Physics Start Simulate!!!" << std::endl;
    bSimulating = true;
}

void MPhysicsEngine::Release()
{
    MeshComponents.clear();
}

void MPhysicsEngine::Temp(std::shared_ptr<MMeshComponent> InMeshComp)
{
    if (InMeshComp)
    {
        MeshComponents.push_back(InMeshComp);
    }
}