#include "Physics.h"

#include "MoonEngine.h"
#include "Renderer.h"
#include "MeshComponent.h"

void MPhysicsEngine::StartSimulate(MWorld* InWorld)
{
    // InWorld->GetName() + " Start Physics Simulate!!!"
    //std::cout << "Physics Start Simulate!!!" << std::endl;
    bSimulating = true;
}

void MPhysicsEngine::Render()
{
    if (getRenderer() /*&& getRenderer()->bDrawCollision*/)
    {
        for (auto& [PrimitiveID, ComponentPhysicsObject] : PhysicsObjects)
        {
            for (auto& PhysicsObject : ComponentPhysicsObject)
            {
                PhysicsObject->Render();
            }
        }
    }
}

void MPhysicsEngine::Release()
{
    Super::Release();
    //MeshComponents.clear();
}

void MPhysicsEngine::AddMeshComponent(std::shared_ptr<MMeshComponent> InMeshComp)
{
    if (InMeshComp)
    {
        //MeshComponents.push_back(InMeshComp);
    }
}

void MPhysicsEngine::RemoveComponent(std::shared_ptr<MMeshComponent> InMeshComp)
{
    if (InMeshComp)
    {
        //MeshComponents.erase()
        //std::remove(MeshComponents.begin(), MeshComponents.end(), InMeshComp);
    }

    // 메시가 가진 오브젝트를 찾음
    uint32 PrimitiveID = InMeshComp->GetPrimitiveID();

    PhysicsObjects.erase(PrimitiveID);

    // 방법 2
    //auto Iter = PhysicsObjects.find(PrimitiveID);
    //if (Iter != PhysicsObjects.end())
    //{
    //    std::shared_ptr<MPhysicsObject>& PhysicsObject = Iter->second;
    //    PhysicsObject->Remove();
    //}

}