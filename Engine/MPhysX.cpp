#include "Include.h"
#include "MPhysX.h"
#include "NvidiaPhysX/extensions/PxDefaultCpuDispatcher.h"
#include "NvidiaPhysX/gpu/PxGpu.h"

#define PVD_HOST "127.0.0.1"

PxDefaultErrorCallback PhysXX::g_DefaultErrorCallback;
PxDefaultAllocator PhysXX::g_DefaultAllocator;

PhysXX::PhysXX()
{
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

	Material = Physics->createMaterial(0.0f, 0.0f, 0.0f);

	//Plane = PxCreatePlane(*Physics, PxPlane(0, 1, 0, 0), *Material);
	//PxShape* PalneShape = Physics->createShape(PxPlaneGeometry(), *Material);
	//Plane->attachShape(*PalneShape);
	//Scene->addActor(*Plane);
    //PxPhysicsGpu* PhysxGpu = PxGetPhysicsGpu();
}

PhysXX::~PhysXX()
{
}

void PhysXX::Release()
{
    Material->release();
    Scene->release();
    CpuDispatcher->release();

    Physics->release();
    VisualDebugger->release();
    Transport->release();
    CudaContextManager->release();
    Foundation->release();
}

PxPhysics* PhysXX::operator->()
{
	return Physics;
}

void PhysXX::Update(float deltaTime)
{
	if (Scene == nullptr)
	{
		return;
	}

	Scene->simulate(deltaTime);
	Scene->fetchResults(true);
    Scene->fetchResultsParticleSystem();
}

bool PhysXX::CreateConvex(const std::vector<Vec3>& Vertices, PxConvexMesh** ConvexMesh)
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
    PxCookConvexMesh(CookingParams, ConvexDesc, buf);

    PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
    *ConvexMesh = Physics->createConvexMesh(input);

	return true;
}
