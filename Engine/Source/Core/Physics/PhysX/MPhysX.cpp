#include "Include.h"
#include "MPhysX.h"
#include "NvidiaPhysX/extensions/PxDefaultCpuDispatcher.h"
#include "NvidiaPhysX/gpu/PxGpu.h"
#include "Core/StaticMesh/StaticMesh.h"

#define PVD_HOST "127.0.0.1"

MPhysX::MPhysX()
{
    static PxDefaultAllocator g_DefaultAllocator;
    static PxDefaultErrorCallback g_DefaultErrorCallback;
	Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_DefaultAllocator, g_DefaultErrorCallback);

	// 비주얼 디버거 연결을 위한 작업
	VisualDebugger = PxCreatePvd(*Foundation);
	Transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10000);
	VisualDebugger->connect(*Transport, PxPvdInstrumentationFlag::eALL);

	// 피직스
	Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *Foundation, PxTolerancesScale(), true, VisualDebugger);

    PxCudaContextManagerDesc CudaContextManagerDesc;
    CudaContextManagerDesc.graphicsDevice = nullptr; // GPU사용하지 않음
    CudaContextManager = PxCreateCudaContextManager(*Foundation, CudaContextManagerDesc, PxGetProfilerCallback());

	CpuDispatcher = PxDefaultCpuDispatcherCreate(4);
    PxSceneDesc SceneDesc = { Physics->getTolerancesScale() };
    SceneDesc.gravity = { 0.f, -9.8f, 0.f };
	SceneDesc.cpuDispatcher = CpuDispatcher;
	SceneDesc.filterShader = PxDefaultSimulationFilterShader;
    SceneDesc.cudaContextManager = CudaContextManager;
    SceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
    SceneDesc.broadPhaseType = PxBroadPhaseType::eGPU;
	Scene = Physics->createScene(SceneDesc);
	Scene->getScenePvdClient()->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
}

MPhysX::~MPhysX()
{
}

void MPhysX::Release()
{
    Scene->release();
    CpuDispatcher->release();

    Physics->release();
    VisualDebugger->release();
    Transport->release();
    CudaContextManager->release();
    Foundation->release();
}

PxPhysics* MPhysX::operator->()
{
	return Physics;
}

void MPhysX::Update(float deltaTime)
{
	if (Scene == nullptr)
	{
		return;
	}

	Scene->simulate(deltaTime);
	Scene->fetchResults(true);
    Scene->fetchResultsParticleSystem();
}

bool MPhysX::CreateConvex(const std::vector<Vec3>& Vertices, PxConvexMesh** ConvexMesh)
{
	PxConvexMeshDesc ConvexDesc = {};
	ConvexDesc.setToDefault();
	ConvexDesc.points.count = CastValue<uint32>(Vertices.size());
	ConvexDesc.points.stride = sizeof(PxVec3);
	ConvexDesc.points.data = reinterpret_cast<const PxVec3*>(&Vertices[0]);
	ConvexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxDefaultMemoryOutputStream buf;
    PxTolerancesScale ToleranceScale;
    PxCookingParams CookingParams(ToleranceScale);
    CookingParams.buildGPUData = true;
    if (PxCookConvexMesh(CookingParams, ConvexDesc, buf))
    {
        PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
        *ConvexMesh = Physics->createConvexMesh(input);

        return true;
    }

    return false;
}

bool MPhysX::AddPhysicsObject(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject)
{
    std::shared_ptr<MPhysXObject> NewObject = std::make_shared<MPhysXObject>(InData.Mesh, InData.PhysicsType);

    // ConvexMesh 있는 거 쓰고, 없으면 만듬 -> 작업 중
    PxMaterial* PhysxMaterial = Physics->createMaterial(0.5f, 0.5f, 0.f);
    PxConvexMesh* ConvexMesh = nullptr;
    CreateConvex(InData.Mesh->GetAllVertexPosition(), &ConvexMesh);

    // Shape 있는 거 쓰고, 없으면 만듬 -> 작업 중
    PxConvexMeshGeometry Geometry = PxConvexMeshGeometry(ConvexMesh);
    PxShape* Shape = Physics->createShape(Geometry, *PhysxMaterial, true);
    NewObject->AttachShape(Shape);

    if (NewObject->IsValid() == false)
    {
        return false;
    }

    OutPhysicsObject = NewObject;

    return true;
}

MPhysXObject::MPhysXObject(std::shared_ptr<StaticMesh> InMesh, EPhysicsType InPhysicsType)
    : MPhysicsObject(nullptr, InMesh)
{
    switch (InPhysicsType)
    {
    case EPhysicsType::Static:
        RigidStatic = GetPhysX()->createRigidStatic(PxTransform(PxVec3(0.f, 0.f, 0.f)));
        break;
    case EPhysicsType::Dynamic:
        RigidDynamic = GetPhysX()->createRigidDynamic(PxTransform(PxVec3(0.f, 0.f, 0.f), PxQuat(PxIdentity)));
        SetMass(1.f);   // DefaultMass
        //PhysXRigidDynamic->setAngularDamping(1.f);
        break;
    }

    if (PxRigidActor* RigidActor = GetRigidActor())
    {
        GetScene()->addActor(*RigidActor);
    }
}

bool MPhysXObject::IsSimulating()
{
    if (PxRigidActor* RigidActor = GetRigidActor())
    {
        return RigidActor->getActorFlags().isSet(PxActorFlag::eDISABLE_SIMULATION) == false;
    }

    return false;
}

void MPhysXObject::SetSimulate(bool bEnable)
{
    PxRigidActor* RigidActor = GetRigidActor();
    if (RigidActor == nullptr)
    {
        return;
    }

    RigidActor->setActorFlag(PxActorFlag::eDISABLE_SIMULATION, !bEnable);
}

void MPhysXObject::SetMass(float InMass)
{
    if (RigidDynamic)
    {
        RigidDynamic->setMass(InMass);
    }
}

void MPhysXObject::SetPos(const Vec3& InPos)
{
    if (PxRigidActor* RigidActor = GetRigidActor())
    {
        PxTransform Transform = RigidActor->getGlobalPose();
        Transform.p.x = InPos.x;
        Transform.p.y = InPos.y;
        Transform.p.z = InPos.z;
        RigidActor->setGlobalPose(Transform);
    }
}

void MPhysXObject::SetScale(const Vec3& InScale)
{
    if (Shape == nullptr)
    {
        return;
    }

    PxRigidActor* PhysXRigidActor = GetRigidActor();
    PhysXRigidActor->detachShape(*Shape);

    PxGeometryHolder holder = Shape->getGeometry();
    holder.convexMesh().scale.scale = PxVec3(InScale.x, InScale.y, InScale.z);
    Shape->setGeometry(holder.any());

    PhysXRigidActor->attachShape(*Shape);

    if (RigidDynamic)
    {
        if (std::shared_ptr<StaticMesh> Mesh = MeshCache.lock())
        {
            Vec3 CenterPos = Mesh->GetCenterPos();
            CenterPos.x *= InScale.x;
            CenterPos.y *= InScale.y;
            CenterPos.z *= InScale.z;
            RigidDynamic->setCMassLocalPose(PxTransform(CenterPos.x, CenterPos.y, CenterPos.z));
        }
    }
}

void MPhysXObject::SetGravity(bool bInGravity)
{
    if (RigidDynamic)
    {
        RigidDynamic->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, bInGravity);
        RigidDynamic->setAngularVelocity(PxVec3(0.f, 0.f, 0.f));
        RigidDynamic->wakeUp();
    }
}

void MPhysXObject::AddForce(const Vec3& InForce)
{
    if (RigidDynamic)
    {
        RigidDynamic->addForce(PxVec3(InForce.x, InForce.y, InForce.z));
    }
}

void MPhysXObject::SetVelocity(const Vec3& InVelocity)
{
    if (RigidDynamic)
    {
        RigidDynamic->setLinearVelocity(PxVec3(InVelocity.x, InVelocity.y, InVelocity.z));
    }
}

void MPhysXObject::SetAngularVelocity(const Vec3& InVelocity)
{
    if (RigidDynamic)
    {
        RigidDynamic->setAngularVelocity(PxVec3(InVelocity.x, InVelocity.y, InVelocity.z));
    }
}

Vec3 MPhysXObject::GetPhysicsPos()
{
    if (PxRigidActor* RigidActor = GetRigidActor())
    {
        PxTransform PhysXTransform = RigidActor->getGlobalPose();
        return Vec3{ PhysXTransform.p.x, PhysXTransform.p.y, PhysXTransform.p.z };
    }

    return VEC3ZERO;
}

Vec4 MPhysXObject::GetPhysicsRotation()
{
    if (PxRigidActor* RigidActor = GetRigidActor())
    {
        PxTransform PhysXTransform = RigidActor->getGlobalPose();
        return { PhysXTransform.q.x, PhysXTransform.q.y, PhysXTransform.q.z, PhysXTransform.q.w };
    }

    return VEC4ZERO;
}

void MPhysXObject::AttachShape(PxShape* InShape)
{
    if (PxRigidActor* RigidActor = GetRigidActor())
    {
        RigidActor->attachShape(*InShape);
        Shape = InShape;
    }
}

physx::PxRigidActor* MPhysXObject::GetRigidActor()
{
    if (RigidStatic)
    {
        return RigidStatic;
    }

    return RigidDynamic;
}
