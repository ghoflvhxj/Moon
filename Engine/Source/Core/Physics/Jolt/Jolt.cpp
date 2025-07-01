#include "Jolt.h"

#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Memory.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Core/JobSystemSingleThreaded.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Collision//Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/SoftBody/SoftBodyCreationSettings.h"
#include "Jolt/Physics/SoftBody/SoftBodyMotionProperties.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/SoftBody/SoftBodyShape.h"

#include "Renderer.h"
#include "VertexBuffer.h"
#include "PrimitiveComponent.h"
#include "Core/StaticMesh/StaticMesh.h"

using namespace JPH;
using namespace JPH::literals;
using namespace std;

#ifdef JPH_ENABLE_ASSERTS
    static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
    {
        // Print to the TTY
        cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << endl;

        // Breakpoint
        return true;
    };
#endif

static void TraceImpl(const char* inFMT, ...)
{
    // Format the message
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);

    // Print to the TTY
    cout << buffer << endl;
}

// 충돌 오브젝트 레이어
namespace Layers
{
    static constexpr ObjectLayer NON_MOVING = 0;
    static constexpr ObjectLayer MOVING = 1;
    static constexpr ObjectLayer NUM_LAYERS = 2;
};

// 레이어간 충돌 여부를 설정
class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
    virtual bool					ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
    {
        switch (inObject1)
        {
        case Layers::NON_MOVING:
            return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING; // Non moving only collides with moving
        case Layers::MOVING:
            return true; // Moving collides with everything
        default:
            JPH_ASSERT(false);
            return false;
        }
    }
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
    static constexpr BroadPhaseLayer NON_MOVING(0);
    static constexpr BroadPhaseLayer MOVING(1);
    static constexpr uint NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
    BPLayerInterfaceImpl()
    {
        // Create a mapping table from object to broad phase layer
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
    }

    virtual uint					GetNumBroadPhaseLayers() const override
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    virtual BroadPhaseLayer			GetBroadPhaseLayer(ObjectLayer inLayer) const override
    {
        JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
    {
        switch ((BroadPhaseLayer::Type)inLayer)
        {
        case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
        case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
        default:													JPH_ASSERT(false); return "INVALID";
        }
    }
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
    BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
    virtual bool				ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1)
        {
        case Layers::NON_MOVING:
            return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING;
        case Layers::MOVING:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
        }
    }
};

// An example contact listener
class MyContactListener : public ContactListener
{
public:
    // See: ContactListener
    virtual ValidateResult	OnContactValidate(const Body& inBody1, const Body& inBody2, RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override
    {
        cout << "Contact validate callback" << endl;

        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void			OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
    {
        cout << "A contact was added" << endl;
    }

    virtual void			OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
    {
        cout << "A contact was persisted" << endl;
    }

    virtual void			OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
    {
        cout << "A contact was removed" << endl;
    }
};

// An example activation listener
class MyBodyActivationListener : public BodyActivationListener
{
public:
    virtual void		OnBodyActivated(const BodyID& inBodyID, uint64 inBodyUserData) override
    {
        cout << "A body got activated" << endl;
    }

    virtual void		OnBodyDeactivated(const BodyID& inBodyID, uint64 inBodyUserData) override
    {
        cout << "A body went to sleep" << endl;
    }
};


MJoltPhysics::MJoltPhysics()
{
    RegisterDefaultAllocator();

    Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

    Factory::sInstance = new Factory();

    RegisterTypes();

    tempAllocator = new TempAllocatorImpl(32 * 1024 * 1024);

    uint32 MaxConcurrentJobs = thread::hardware_concurrency();
    // Create job system
    jobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, MaxConcurrentJobs - 1);
    // Create single threaded job system for validatingS
    jobSystemValidating = new JobSystemSingleThreaded(cMaxPhysicsJobs);


    JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
    const uint cMaxBodies = 1024;
    const uint cNumBodyMutexes = 0;
    const uint cMaxBodyPairs = 1024;
    const uint cMaxContactConstraints = 1024;

    static BPLayerInterfaceImpl broad_phase_layer_interface;
    static ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
    static ObjectLayerPairFilterImpl object_vs_object_layer_filter;

    physics_system = new PhysicsSystem();
    physics_system->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

    static MyBodyActivationListener body_activation_listener;
    physics_system->SetBodyActivationListener(&body_activation_listener);    

    static  MyContactListener contact_listener;
    physics_system->SetContactListener(&contact_listener);
}

bool MJoltPhysics::AddPhysicsObject(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject)
{
    BodyInterface& bodyInterface = physics_system->GetBodyInterface();
    
    const std::vector<::Vec3>& Vertices = InData.Mesh->GetAllVertexPosition();
    const std::vector<uint32>& Indices = InData.Mesh->GetMeshData(0)->Indices;

    Body* NewBody = nullptr;

    Ref<Shape> NewShape;

    // Make ConvexHull
    {
        std::vector<JPH::Vec3> JPHVertices(Vertices.size());
        for (int i = 0; i < Vertices.size(); ++i)
        {
            JPHVertices[i].SetX(Vertices[i].x);
            JPHVertices[i].SetY(Vertices[i].y);
            JPHVertices[i].SetZ(Vertices[i].z);
            JPHVertices[i].mF32[3] = JPHVertices[i].mF32[2];
        }
        NewShape = ConvexHullShapeSettings(JPHVertices.data(), GetSize(JPHVertices)).Create().Get();
    }
    EMotionType MotionType = InData.PhysicsType == EPhysicsType::Static ? EMotionType::Static : EMotionType::Dynamic;

    // MeshShape
    //if (MotionType == EMotionType::Static)
    //{
    //    JPH::VertexList vertexList;
    //    for (int i = 0; i < Vertices.size(); ++i)
    //    {
    //        vertexList.push_back(Float3(Vertices[i].x, Vertices[i].y, Vertices[i].z));
    //    }

    //    IndexedTriangleList triangleList;
    //    uint32 IndexLoopNum = GetSize(Indices) / 3;
    //    for (uint32 i = 0; i < IndexLoopNum; ++i)
    //    {
    //        triangleList.push_back(IndexedTriangle(Indices[i * 3], Indices[i * 3 + 1], Indices[i * 3 + 2]));
    //    }
    //    NewShape = MeshShapeSettings(vertexList, triangleList).Create().Get();
    //}

    BodyID bodyID = bodyInterface.CreateAndAddBody(BodyCreationSettings(NewShape.GetPtr(), RVec3(0.f, 0.f, 0.f), QuatArg::sIdentity(), MotionType, Layers::NON_MOVING), EActivation::Activate);

    std::shared_ptr<MJoltPhysicsObject> NewPhysicsObject = std::make_shared<MJoltPhysicsObject>(InData);
    NewPhysicsObject->SetBodyID(bodyID);

    OutPhysicsObject = NewPhysicsObject;

    return true;
}

bool MJoltPhysics::AddCloth(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject)
{
    BodyInterface& bodyInterface = physics_system->GetBodyInterface();

    const std::vector<::Vec3>& Vertices = InData.Mesh->GetAllVertexPosition();
    const std::vector<uint32>& Indices = InData.Mesh->GetMeshData(0)->Indices;

    SoftBodySharedSettings* NewSharedSettings = new SoftBodySharedSettings();
    for (const ::Vec3& MeshVertex : Vertices)
    {
        SoftBodySharedSettings::Vertex NewVertex;
        JPH::Vec3 Position(MeshVertex.x, MeshVertex.y, MeshVertex.z);
        Position.StoreFloat3(&NewVertex.mPosition);
        NewVertex.mInvMass = 1.f;
        NewSharedSettings->mVertices.push_back(NewVertex);
    }

    uint32 IndexLoopNum = GetSize(Indices) / 3;
    for (uint32 i = 0; i < IndexLoopNum; ++i)
    {
        SoftBodySharedSettings::Face NewFace;
        NewFace.mVertex[0] = Indices[i * 3];
        NewFace.mVertex[1] = Indices[i * 3 + 1];
        NewFace.mVertex[2] = Indices[i * 3 + 2];
        NewSharedSettings->AddFace(NewFace);
    }
    SoftBodySharedSettings::VertexAttributes inVertexAttributes = { 1.0e-5f, 1.0e-5f, 1.0e-5f };
    NewSharedSettings->CreateConstraints(&inVertexAttributes, 1);
    NewSharedSettings->Optimize();

    SoftBodyCreationSettings Cloth(NewSharedSettings, JPH::Vec3(0.f, 5.f, 0.f), QuatArg::sIdentity(), Layers::MOVING);
    BodyID bodyId =  bodyInterface.CreateAndAddSoftBody(Cloth, EActivation::Activate);

    std::shared_ptr<MJoltPhysicsObject> JoltPhysicsObject = std::make_shared<MJoltPhysicsObject>(InData);
    JoltPhysicsObject->SetBodyID(bodyId);

    OutPhysicsObject = JoltPhysicsObject;

    SoftBodyObjects.push_back(OutPhysicsObject);

    return true;
}

void MJoltPhysics::Update(float deltaTime)
{
    // 시뮬레이션
    physics_system->Update(deltaTime, 4, tempAllocator, jobSystem);

    // SoftBody의 정점위치 갱신
    for (auto& SoftBodyObject : SoftBodyObjects)
    {
        std::shared_ptr<MJoltPhysicsObject> PhysicObject = std::static_pointer_cast<MJoltPhysicsObject>(SoftBodyObject.lock());
        if (PhysicObject == nullptr)
        {
            continue;
        }

        Array<SoftBodyMotionProperties::Vertex> SoftBodyVertices;
        BodyLockRead lock(physics_system->GetBodyLockInterface(), PhysicObject->GetBodyID());
        if (lock.Succeeded())
        {
            const Body& SoftBody = lock.GetBody();
            const SoftBodyMotionProperties* motionProperties = static_cast<const SoftBodyMotionProperties*>(SoftBody.GetMotionProperties());
            
            SoftBodyVertices = motionProperties->GetVertices();
        }

        std::vector<::Vertex> Vertices = PhysicObject->GetMesh()->GetMeshData(0)->Vertices;
        for (uint32 i = 0; i < GetSize(Vertices); ++i)
        {
            Vertices[i].Pos.x = SoftBodyVertices[i].mPosition.GetX();
            Vertices[i].Pos.y = SoftBodyVertices[i].mPosition.GetY();
            Vertices[i].Pos.z = SoftBodyVertices[i].mPosition.GetZ();
        }
    
        PhysicObject->UpdateVertices(Vertices);
    }
}

void MJoltPhysics::Release()
{
    delete physics_system;
    delete jobSystemValidating;
    delete jobSystem;
    delete tempAllocator;
    delete Factory::sInstance;
}

MJoltPhysicsObject::MJoltPhysicsObject(FPhysicsConstructData& InData)
    : MPhysicsObject(InData.PrimitiveComponent, InData.Mesh)
{

}

void MJoltPhysicsObject::UpdateVertices(std::vector<::Vertex>& InVertices)
{
    std::shared_ptr<MVertexBuffer> VertexBuffer = g_pRenderer->GetVertexBuffer(GetPrimitiveComponent()->GetPrimitiveID());
    VertexBuffer->Update(InVertices.data());
}

bool MJoltPhysicsObject::IsSimulating()
{
    return GetPhysicsSystem()->GetBodyInterface().IsActive(BodyIDCache);
}

void MJoltPhysicsObject::SetSimulate(bool bEnable)
{
    BodyInterface& bodyInterface = GetPhysicsSystem()->GetBodyInterface();
    if (bEnable)
    {
        bodyInterface.ActivateBody(BodyIDCache);
    }
    else
    {
        bodyInterface.DeactivateBody(BodyIDCache);
    }
}

void MJoltPhysicsObject::SetMass(float InMass)
{

}

void MJoltPhysicsObject::SetPos(const ::Vec3& InPos)
{
    GetPhysicsSystem()->GetBodyInterface().SetPosition(BodyIDCache, RVec3Arg(InPos.x, InPos.y, InPos.z), EActivation::DontActivate);
}

void MJoltPhysicsObject::SetScale(const ::Vec3& InScale)
{
    Ref<Shape> scaledShape;

    BodyLockRead Lock(GetPhysicsSystem()->GetBodyLockInterface(), BodyIDCache);
    if (Lock.Succeeded())
    {
        scaledShape = Lock.GetBody().GetShape()->ScaleShape(JPH::Vec3(InScale.x, InScale.y, InScale.z)).Get();
    }
    Lock.ReleaseLock();

    GetPhysicsSystem()->GetBodyInterface().SetShape(BodyIDCache, scaledShape.GetPtr(), false, EActivation::Activate);

}

void MJoltPhysicsObject::SetGravity(bool bGravity)
{

}

void MJoltPhysicsObject::AddForce(const ::Vec3& InForce)
{

}

void MJoltPhysicsObject::SetVelocity(const ::Vec3& InVelocity)
{
    GetPhysicsSystem()->GetBodyInterface().SetLinearVelocity(BodyIDCache, RVec3Arg(InVelocity.x, InVelocity.y, InVelocity.z));
}

void MJoltPhysicsObject::SetAngularVelocity(const ::Vec3& InVelocity)
{

}

::Vec3 MJoltPhysicsObject::GetPhysicsPos()
{
    JPH::Vec3 OutPos = GetPhysicsSystem()->GetBodyInterface().GetPosition(BodyIDCache);
    return { OutPos.GetX(), OutPos.GetY(), OutPos.GetZ() };
}

::Vec4 MJoltPhysicsObject::GetPhysicsRotation()
{
    return VEC4ZERO;
}
