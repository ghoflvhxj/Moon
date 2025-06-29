#include "Jolt.h"

#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Memory.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Core/JobSystemSingleThreaded.h"

#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics//Collision/Shape/ConvexHullShape.h"

#include "StaticMeshComponent.h"

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

    physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

    static MyBodyActivationListener body_activation_listener;
    physics_system.SetBodyActivationListener(&body_activation_listener);    

    static  MyContactListener contact_listener;
    physics_system.SetContactListener(&contact_listener);

    /*
    BodyInterface& body_interface = physics_system.GetBodyInterface();
    BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));
    floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

    // Create the shape
    ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
    ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

    // Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
    BodyCreationSettings floor_settings(floor_shape, RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

    // Create the actual rigid body
    Body* floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

    // Add it to the world
    body_interface.AddBody(floor->GetID(), EActivation::DontActivate);
    */
}

bool MJoltPhysics::AddPhysicsObject(std::shared_ptr<StaticMesh> InMesh, EPhysicsType InPhysicsType, std::unique_ptr<MPhysicsObject>& OutPhysicsObject)
{
    BodyInterface& bodyInterface = physics_system.GetBodyInterface();
    
    const std::vector<::Vec3>& Vertices = InMesh->GetAllVertexPosition();
    std::vector<JPH::Vec3> JPHVertices(Vertices.size());
    for (int i = 0; i < Vertices.size(); ++i)
    {
        JPHVertices[i].SetX(Vertices[i].x);
        JPHVertices[i].SetY(Vertices[i].y);
        JPHVertices[i].SetZ(Vertices[i].z);
        JPHVertices[i].mF32[3] = JPHVertices[i].mF32[2];
    }

    Ref<Shape> NewShape = ConvexHullShapeSettings(JPHVertices.data(), GetSize(JPHVertices)).Create().Get();

    EMotionType MotionType = InPhysicsType == EPhysicsType::Static ? EMotionType::Static : EMotionType::Dynamic;
    Body* NewBody = bodyInterface.CreateBody(BodyCreationSettings(NewShape.GetPtr(), RVec3(0.f, 0.f, 0.f), QuatArg::sIdentity(), MotionType, Layers::NON_MOVING));

    std::unique_ptr<MJoltPhysicsObject> NewPhysicsObject = std::make_unique<MJoltPhysicsObject>(InMesh, InPhysicsType);
    NewPhysicsObject->AttachShape(NewShape);
    NewPhysicsObject->SetBody(NewBody);
   
    bodyInterface.SetGravityFactor(NewBody->GetID(), 0.98f);
    bodyInterface.AddBody(NewBody->GetID(), EActivation::Activate);

    OutPhysicsObject = std::move(NewPhysicsObject);

    return true;
}

void MJoltPhysics::Update(float deltaTime)
{
    physics_system.Update(deltaTime, 4, tempAllocator, jobSystem);
}

MJoltPhysicsObject::MJoltPhysicsObject(std::shared_ptr<StaticMesh> InMesh, EPhysicsType InPhysicsType)
    : MPhysicsObject(InMesh)
{

}

bool MJoltPhysicsObject::IsSimulating()
{
    if (BodyCache)
    {
        return GetPhysicsSystem()->GetBodyInterface().IsActive(BodyCache->GetID());
    }

    return false;
}

void MJoltPhysicsObject::SetSimulate(bool bEnable)
{
    if (BodyCache == nullptr)
    {
        return;
    }

    BodyID bodyId = BodyCache->GetID();

    BodyInterface& bodyInterface = GetPhysicsSystem()->GetBodyInterface();
    if (bEnable)
    {
        bodyInterface.ActivateBody(bodyId);
    }
    else
    {
        bodyInterface.DeactivateBody(bodyId);
    }
}

void MJoltPhysicsObject::SetMass(float InMass)
{

}

void MJoltPhysicsObject::SetPos(const ::Vec3& InPos)
{
    if (BodyCache)
    {
        GetPhysicsSystem()->GetBodyInterface().SetPosition(BodyCache->GetID(), RVec3Arg(InPos.x, InPos.y, InPos.z), EActivation::DontActivate);
    }
}

void MJoltPhysicsObject::SetScale(const ::Vec3& InScale)
{
    if (ShapeCache && BodyCache)
    {
        Ref<Shape> scaledShape = ShapeCache->ScaleShape(JPH::Vec3(InScale.x, InScale.y, InScale.z)).Get();
        GetPhysicsSystem()->GetBodyInterface().SetShape(BodyCache->GetID(), scaledShape.GetPtr(), false, EActivation::Activate);
    }
}

void MJoltPhysicsObject::SetGravity(bool bGravity)
{

}

void MJoltPhysicsObject::AddForce(const ::Vec3& InForce)
{

}

void MJoltPhysicsObject::SetVelocity(const ::Vec3& InVelocity)
{
    if (BodyCache)
    {
        GetPhysicsSystem()->GetBodyInterface().SetLinearVelocity(BodyCache->GetID(), RVec3Arg(InVelocity.x, InVelocity.y, InVelocity.z));
    }
}

void MJoltPhysicsObject::SetAngularVelocity(const ::Vec3& InVelocity)
{

}

::Vec3 MJoltPhysicsObject::GetPhysicsPos()
{
    if (BodyCache)
    {
        JPH::Vec3 OutPos = GetPhysicsSystem()->GetBodyInterface().GetPosition(BodyCache->GetID());
        return { OutPos.GetX(), OutPos.GetY(), OutPos.GetZ() };
    }

    return VEC3ZERO;
}

::Vec4 MJoltPhysicsObject::GetPhysicsRotation()
{
    return VEC4ZERO;
}
