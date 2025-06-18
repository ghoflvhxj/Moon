#include "Include.h"
#include "MPhysX.h"

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

	// 쿠킹
	Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *Foundation, PxCookingParams(Physics->getTolerancesScale()));


	PxSceneDesc SceneDesc = { Physics->getTolerancesScale() };
	SceneDesc.gravity = { 0.f, -9.8f, 0.f };

	CpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	SceneDesc.cpuDispatcher = CpuDispatcher;
	SceneDesc.filterShader = PxDefaultSimulationFilterShader;
	Scene = Physics->createScene(SceneDesc);
	Scene->getScenePvdClient()->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);

	Material = Physics->createMaterial(0.0f, 0.0f, 0.0f);

	//Plane = PxCreatePlane(*Physics, PxPlane(0, 1, 0, 0), *Material);
	//PxShape* PalneShape = Physics->createShape(PxPlaneGeometry(), *Material);
	//Plane->attachShape(*PalneShape);
	//Scene->addActor(*Plane);
}

PhysXX::~PhysXX()
{
}

void PhysXX::Release()
{
    Material->release();
    Scene->release();
    CpuDispatcher->release();

    Cooking->release();
    Physics->release();
    VisualDebugger->release();
    Transport->release();
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
}

bool PhysXX::CreateConvex(const std::vector<Vec3>& Vertices, PxConvexMesh** ConvexMesh)
{
	PxConvexMeshDesc ConvexDesc = {};
	ConvexDesc.setToDefault();
	ConvexDesc.points.count = CastValue<uint32>(Vertices.size());
	ConvexDesc.points.stride = sizeof(PxVec3);
	ConvexDesc.points.data = reinterpret_cast<const PxVec3*>(&Vertices[0]);
	ConvexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

//#ifdef _DEBUG
//	// mesh should be validated before cooking without the mesh cleaning
//	bool res = Cooking->validateConvexMesh(ConvexDesc);
//	PX_ASSERT(res);
//#endif
//
//	*ConvexMesh = Cooking->createConvexMesh(ConvexDesc, Physics->getPhysicsInsertionCallback());

	PxDefaultMemoryOutputStream buf;
	if (!Cooking->cookConvexMesh(ConvexDesc, buf))
		return false;

	PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
	*ConvexMesh = Physics->createConvexMesh(input);

	return true;
}
