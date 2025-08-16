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
#include "Jolt/Physics/Collision/Shape/EmptyShape.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/SoftBody/SoftBodyCreationSettings.h"
#include "Jolt/Physics/SoftBody/SoftBodyMotionProperties.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/SoftBody/SoftBodyShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Constraints/FixedConstraint.h"
#include "Jolt/ObjectStream/ObjectStreamTextOut.h"
#include "Jolt/ObjectStream/ObjectStreamTextIn.h"

#include "Renderer.h"
#include "Vertex.h"
#include "VertexBuffer.h"
#include "PrimitiveComponent.h"
#include "Mesh/Mesh.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"
#include "DynamicMeshComponent.h"
#include "MainGame.h"
#include "Core/FileSystem.h"
#include <DirectXMath.h>

using namespace JPH;
using namespace JPH::literals;
using namespace std;
using namespace DirectX;

constexpr float SoftBodyMagicNum = 1.f;
constexpr char* BoneName = "bone014";

#define SKIN 0
#undef min

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

void MJoltPhysics::StartSimulate()
{
    MPhysicsEngine::StartSimulate();

    for (auto WeakMeshComp : MeshComponents)
    {
        auto MeshComp = WeakMeshComp.lock();

        std::shared_ptr<StaticMesh> Mesh = MeshComp->GetMesh();
        if (Mesh == nullptr)
        {
            continue;
        }

        std::shared_ptr<MPhysics> Physics = Mesh->GetPhysics();
        if (Physics == nullptr)
        {
            continue;
        }

        std::string Path = WStringToString(Physics->GetAssetPath());

        BodyInterface& bodyInterface = physics_system->GetBodyInterface();

        ConvexHullShapeSettings* Test = nullptr;
        std::stringstream ss;
        ObjectStreamTextIn StreamIn = JPH::ObjectStreamTextIn(ss);
        StreamIn.sReadObject(Path.c_str(), Test);

        ::Vec3 CompPos = MeshComp->getWorldTranslation();
        ::Vec3 CompRot = MeshComp->getRotation();
        ::Vec3 CompScale = MeshComp->getScale();

        Ref<Shape> NewShape = Test->Create().Get();
        JPH::Vec3 Pos = { CompPos.x, CompPos.y, CompPos.z };
        Quat Rot = Quat::sEulerAngles(Vec3Arg{ CompRot.x, CompRot.y, CompRot.z });

        EMotionType MotionType = ConvertPhysicsType(MeshComp->GetPhysicsType());

        ObjectLayer Layer = (MotionType == EMotionType::Static) ? Layers::NON_MOVING : Layers::MOVING;
        EActivation Activation = EActivation::DontActivate;
        if (MeshComp->IsPhysicsEnable() && bSimulating)
        {
            Activation = EActivation::Activate;
        }
        BodyID NewBodyID = bodyInterface.CreateAndAddBody(BodyCreationSettings(NewShape, Pos, Rot, MotionType, Layer), Activation);

        FPhysicsConstructData Data;
        Data.Mesh = Mesh;
        Data.PrimitiveComponent = MeshComp;

        std::shared_ptr<MJoltPhysicsObject> NewPhysicsObject = std::make_shared<MJoltPhysicsObject>(Data);
        NewPhysicsObject->SetBodyID(NewBodyID);
        NewPhysicsObject->SetScale(CompScale);

        MeshComp->PhysicsObject = NewPhysicsObject;

        // ReadObject가 new를 이용해 Object를 생성하니, 삭제도 해줘야 함
        delete Test;
    }
}

void MJoltPhysics::LoadTest()
{
    ConvexHullShapeSettings* Test = nullptr;

    std::stringstream ss;
    JPH::ObjectStreamTextIn StreamIn = JPH::ObjectStreamTextIn(ss);
    StreamIn.sReadObject("D:\\Git\\Moon\\JoltTest.physics", Test);

    Ref<Shape> NewShape = Test->Create().Get();
}

void MJoltPhysics::SaveTest(std::shared_ptr<StaticMesh> InMesh)
{
    if (InMesh == nullptr)
    {
        return;
    }

    BodyInterface& bodyInterface = physics_system->GetBodyInterface();

    const std::vector<::Vec3>& Vertices = InMesh->GetAllVertexPosition();
    const std::vector<uint32>& Indices = InMesh->GetMeshData(0)->Indices;

    std::vector<JPH::Vec3> JPHVertices(Vertices.size());
    for (int i = 0; i < Vertices.size(); ++i)
    {
        JPHVertices[i].SetX(Vertices[i].x);
        JPHVertices[i].SetY(Vertices[i].y);
        JPHVertices[i].SetZ(Vertices[i].z);
        JPHVertices[i].mF32[3] = JPHVertices[i].mF32[2];
    }

    std::filesystem::path Path = MFIleSystem::AbsolutePath(InMesh->GetAssetPath());
    Path.replace_extension("physics");

    // 저장 테스트
    std::stringstream ss;
    JPH::ObjectStreamTextOut streamOut = JPH::ObjectStreamTextOut(ss);
    streamOut.sWriteObject(Path.string().c_str(), JPH::ObjectStream::EStreamType::Text, ConvexHullShapeSettings(JPHVertices.data(), GetSize(JPHVertices)));
    
    std::shared_ptr<MPhysics> NewPhysics = std::make_shared<MPhysics>();
    NewPhysics->SetAssetPath(Path);

    InMesh->SetPhysics(NewPhysics);
}

void MJoltPhysics::MakeConvexHull(FPhysicsConstructData& InData)
{
    uint32 PrimitiveID = InData.PrimitiveComponent->GetPrimitiveID();

    BodyInterface& bodyInterface = physics_system->GetBodyInterface();

    const std::vector<::Vec3>& Vertices = InData.Mesh->GetAllVertexPosition();
    const std::vector<uint32>& Indices = InData.Mesh->GetMeshData(0)->Indices;

    std::vector<JPH::Vec3> JPHVertices(Vertices.size());
    for (int i = 0; i < Vertices.size(); ++i)
    {
        JPHVertices[i].SetX(Vertices[i].x);
        JPHVertices[i].SetY(Vertices[i].y);
        JPHVertices[i].SetZ(Vertices[i].z);
        JPHVertices[i].mF32[3] = JPHVertices[i].mF32[2];
    }

    Ref<Shape> NewShape = ConvexHullShapeSettings(JPHVertices.data(), GetSize(JPHVertices)).Create().Get();
    JPH::Vec3 Pos = { InData.Pos.x, InData.Pos.y, InData.Pos.z };
    JPH::Quat Rot = QuatArg::sIdentity();
    EMotionType MotionType = ConvertPhysicsType(InData.PhysicsType);
    JPH::ObjectLayer Layer = (MotionType == EMotionType::Static) ? Layers::NON_MOVING : Layers::MOVING;
    JPH::EActivation Activation = bSimulating ? EActivation::Activate : EActivation::DontActivate;
    BodyID NewBodyID = bodyInterface.CreateAndAddBody(BodyCreationSettings(NewShape.GetPtr(), Pos, Rot, MotionType, Layer), EActivation::DontActivate);
}

JPH::EMotionType MJoltPhysics::ConvertPhysicsType(EPhysicsType InType)
{
    switch (InType)
    {
    case EPhysicsType::Static:
        return JPH::EMotionType::Static;
    case EPhysicsType::Dynamic:
        return JPH::EMotionType::Dynamic;
    case EPhysicsType::Kinematic:
        return JPH::EMotionType::Kinematic;
    default:
        return JPH::EMotionType::Static;
    }
}

bool MJoltPhysics::AddPhysicsObject(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject)
{
    BodyInterface& bodyInterface = physics_system->GetBodyInterface();
    Ref<Shape> NewShape;
    BodyID NewBodyID;

    if (InData.bCapsule == false)
    {


        const std::vector<::Vec3>& Vertices = InData.Mesh->GetAllVertexPosition();
        const std::vector<uint32>& Indices = InData.Mesh->GetMeshData(0)->Indices;

        //if (Vertices.empty())
        //{
        //    return false;
        //}


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
            EMotionType MotionType = InData.PhysicsType == EPhysicsType::Static ? EMotionType::Static : EMotionType::Dynamic;
            NewBodyID = bodyInterface.CreateAndAddBody(BodyCreationSettings(NewShape.GetPtr(), RVec3(0.f, 0.f, 0.f), QuatArg::sIdentity(), MotionType, Layers::NON_MOVING), EActivation::Activate);
        }
    }

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

    // 캡슐
    if (InData.bCapsule)
    {
        RefConst<Shape> big_capsule = new CapsuleShape(3.5f, 0.13f);
        auto b = BodyCreationSettings(big_capsule, RVec3(0, 0.f, 0), Quat::sEulerAngles(Vec3Arg(0.f, 0.f, 0.f)), EMotionType::Kinematic, Layers::MOVING);
        b.mMassPropertiesOverride.mMass = 0.f;
        b.mGravityFactor = 0.f;
        NewBodyID = bodyInterface.CreateAndAddBody(b, EActivation::Activate);
    }

    std::shared_ptr<MJoltPhysicsObject> NewPhysicsObject = std::make_shared<MJoltPhysicsObject>(InData);
    NewPhysicsObject->SetBodyID(NewBodyID);
    OutPhysicsObject = NewPhysicsObject;

    return true;
}

bool MJoltPhysics::AddCloth(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject)
{
    BodyInterface& bodyInterface = physics_system->GetBodyInterface();

    const std::vector<::Vec3>& Vertices = InData.Mesh->GetAllVertexPosition();
    const std::vector<uint32>& Indices = InData.Mesh->GetMeshData(0)->Indices;

    SoftBodySharedSettings* NewSharedSettings = new SoftBodySharedSettings();

    // 점의 위치를 채움
    for (const ::Vec3& VtxPos : Vertices)
    {
        SoftBodySharedSettings::Vertex NewVertex;
        NewVertex.mPosition = { VtxPos.x, VtxPos.y, VtxPos.z };
        NewVertex.mInvMass = 1.f;
        NewSharedSettings->mVertices.push_back(NewVertex);
    }

    // 면을 만들어 줌
    uint32 IndexLoopNum = GetSize(Indices) / 3;
    for (uint32 i = 0; i < IndexLoopNum; ++i)
    {
        SoftBodySharedSettings::Face NewFace;
        NewFace.mVertex[0] = Indices[i * 3];
        NewFace.mVertex[1] = Indices[i * 3 + 1];
        NewFace.mVertex[2] = Indices[i * 3 + 2];
        NewSharedSettings->AddFace(NewFace);
    }

    SoftBodySharedSettings::VertexAttributes inVertexAttributes = { 0.f, 0.f, 0.f, SoftBodySharedSettings::ELRAType::GeodesicDistance };
    NewSharedSettings->CreateConstraints(&inVertexAttributes, 4);
    NewSharedSettings->Optimize();

    SoftBodyCreationSettings Cloth(NewSharedSettings, JPH::Vec3(0.f, 5.f, 0.f), QuatArg::sIdentity(), Layers::MOVING);
    BodyID bodyId =  bodyInterface.CreateAndAddSoftBody(Cloth, EActivation::Activate);

    std::shared_ptr<MJoltPhysicsObject> JoltPhysicsObject = std::make_shared<MJoltPhysicsObject>(InData);
    JoltPhysicsObject->SetBodyID(bodyId);

    OutPhysicsObject = JoltPhysicsObject;

    SoftBodyObjects.push_back(OutPhysicsObject);

    return true;
}

void MJoltPhysics::AddCloth(FPhysicsConstructData& InData, std::vector<FClothData>& ClothDatas, std::shared_ptr<MPhysicsObject>& OutPhysicsObject)
{
    BodyInterface& bodyInterface = physics_system->GetBodyInterface();

    SoftBodySharedSettings* NewSharedSettings = new SoftBodySharedSettings();

    // 중첩을 제거한 버텍스, 인덱스 쌍
    std::unordered_map<FVertexKey, uint32> VertexIndex;

    std::vector<uint32> MeshIndices;
    for (const FClothData& ClothData : ClothDatas)
    {
        MeshIndices.push_back(ClothData.MeshIndex);
        std::shared_ptr<FMeshData> MeshData = InData.Mesh->GetMeshData(ClothData.MeshIndex);
        
        for (uint32 i = 0; i < GetSize(MeshData->Vertices); ++i)
        {
            const ::Vec4& VtxPos = MeshData->Vertices[i].Pos;
            FVertexKey VertexKey = { VtxPos.x, VtxPos.y, VtxPos.z };

            if (VertexIndex.find(VertexKey) != VertexIndex.end())
            {
                continue;
            }

            VertexIndex[VertexKey] = GetSize(NewSharedSettings->mVertices);

            SoftBodySharedSettings::Vertex NewVertex;
            NewVertex.mPosition = { VtxPos.x, VtxPos.y, VtxPos.z };
            NewVertex.mInvMass = VtxPos.y > 1.f ? 0.f : 1.f;
            NewSharedSettings->mVertices.push_back(NewVertex);
        }

        uint32 IndexLoopNum = GetSize(MeshData->Indices) / 3;
        for (uint32 i = 0; i < IndexLoopNum; ++i)
        {
            SoftBodySharedSettings::Face NewFace;

            for (uint32 j = 0; j < 3; ++j)
            {
                uint32 Index = MeshData->Indices[i * 3 + j];
                const ::Vec4& VtxPos = MeshData->Vertices[Index].Pos;

                FVertexKey VertexKey = { VtxPos.x, VtxPos.y, VtxPos.z };
                Index = VertexIndex[VertexKey];

                NewFace.mVertex[j] = Index;
            }

            NewSharedSettings->AddFace(NewFace);
        }
    }

    // 바인드 포즈 역행렬 ---------------------------------------------------------------------------------------------
    auto& Joints = std::static_pointer_cast<DynamicMesh>(InData.Mesh)->GetJoints();
    uint32 JointNum = GetSize(Joints);
    //for (uint32 JointIndex = 0; JointIndex < JointNum; ++JointIndex)
    {
        Mat4& MyMat = Joints[std::static_pointer_cast<DynamicMesh>(InData.Mesh)->GetJointIndex(BoneName)]._globalBindPoseInverseMatrix;
        Mat44 mat = {
            Vec4Arg{MyMat._11, MyMat._12, MyMat._13, MyMat._14},
            Vec4Arg{MyMat._21, MyMat._22, MyMat._23, MyMat._24},
            Vec4Arg{MyMat._31, MyMat._32, MyMat._33, MyMat._34},
            Vec4Arg{MyMat._41, MyMat._42, MyMat._43, MyMat._44}
        };
        //mat = Mat44::sIdentity();
        NewSharedSettings->mInvBindMatrices.emplace_back(0, mat);
    }


#if SKIN == 1
    // 스키닝 제약 ---------------------------------------------------------------------------------------------
    uint32 VertexNum = GetSize(NewSharedSettings->mVertices);
    for (uint32 VertexIndex = 0; VertexIndex < VertexNum; ++VertexIndex)
    {
        SoftBodySharedSettings::Skinned NewSkinConstraint(VertexIndex, NewSharedSettings->mVertices[VertexIndex].mInvMass > 0.0f ? 0.1f : 0.f, 0.1f, 40.f);

        NewSkinConstraint.mWeights[0] = SoftBodySharedSettings::SkinWeight(0, 1.f);

        NewSharedSettings->mSkinnedConstraints.push_back(NewSkinConstraint);
    }
    NewSharedSettings->CalculateSkinnedConstraintNormals();
#else
    // 일반 제약 ---------------------------------------------------------------------------------------------
    SoftBodySharedSettings::VertexAttributes inVertexAttributes = { 0.f, 0.f, 0.f, SoftBodySharedSettings::ELRAType::GeodesicDistance };
    NewSharedSettings->CreateConstraints(&inVertexAttributes, 1);
#endif

    // 바디 생성 ---------------------------------------------------------------------------------------------
    NewSharedSettings->Optimize();
    //JPH::Vec3 ClothPos = JPH::Vec3(InData.Pos.x, InData.Pos.y, InData.Pos.z);
    JPH::Vec3 ClothPos = JPH::Vec3::sZero();
    SoftBodyCreationSettings ClothCreateSetting(NewSharedSettings, ClothPos, QuatArg::sIdentity(), Layers::MOVING);
    ClothCreateSetting.mAllowSleeping = false;
    ClothCreateSetting.mLinearDamping = 0.f;
    BodyID bodyId = bodyInterface.CreateAndAddSoftBody(ClothCreateSetting, EActivation::Activate);

    std::shared_ptr<MJoltPhysicsObject> JoltPhysicsObject = std::make_shared<MJoltPhysicsObject>(InData);
    JoltPhysicsObject->SetBodyID(bodyId);
    JoltPhysicsObject->SetMeshIndices(MeshIndices);
    JoltPhysicsObject->SetVertexIndices(VertexIndex);
    //JoltPhysicsObject->CachePos = InData.Pos;

    OutPhysicsObject = JoltPhysicsObject;
    SoftBodyObjects.push_back(OutPhysicsObject);

    // 스키닝 적용 ---------------------------------------------------------------------------------------------
#if SKIN == 1
    JoltPhysicsObject->SkinVertices(true, tempAllocator);
#endif

    SoftBodyMotionProperties* mp = static_cast<SoftBodyMotionProperties*>(JoltPhysicsObject->GetBody().GetMotionProperties());
    mp->SetUpdatePosition(false);
}

void MJoltPhysics::AddKinematic(FPhysicsConstructData& InData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject)
{
    JPH::Vec3 Pos = JPH::Vec3(InData.Pos.x, InData.Pos.y, InData.Pos.z);

    // 고정을 위한 Kinematic
    EmptyShapeSettings* ShapeSetting = new EmptyShapeSettings();
    BodyCreationSettings JointBodySettings(
        ShapeSetting,
        Pos,
        QuatArg::sIdentity(),
        EMotionType::Kinematic,
        Layers::MOVING
    );

    auto& bodyInterface = physics_system->GetBodyInterface();
    BodyID bodyId = bodyInterface.CreateAndAddBody(JointBodySettings, EActivation::Activate);

    std::shared_ptr<MJoltPhysicsObject> JoltPhysicsObject = std::make_shared<MJoltPhysicsObject>(InData);
    JoltPhysicsObject->SetBodyID(bodyId);

    OutPhysicsObject = JoltPhysicsObject;
}

void MJoltPhysics::Constraint(std::shared_ptr<MPhysicsObject>& Lhs, std::shared_ptr<MPhysicsObject>& Rhs)
{
    std::shared_ptr<MJoltPhysicsObject> lhs = std::static_pointer_cast<MJoltPhysicsObject>(Lhs);
    std::shared_ptr<MJoltPhysicsObject> rhs = std::static_pointer_cast<MJoltPhysicsObject>(Rhs);

    FixedConstraintSettings* ConstraintSetting = new FixedConstraintSettings();
    physics_system->AddConstraint(ConstraintSetting->Create(lhs->GetBody(), rhs->GetBody()));
}

void MJoltPhysics::Update(float deltaTime)
{
    physics_system->Update(deltaTime, 1, tempAllocator, jobSystem);

    // SoftBody의 정점위치 갱신
    for (auto& SoftBodyObject : SoftBodyObjects)
    {
        if (SoftBodyObject.expired())
        {
            continue;
        }

        std::shared_ptr<MJoltPhysicsObject> PhysicObject = std::static_pointer_cast<MJoltPhysicsObject>(SoftBodyObject.lock());

        Array<SoftBodyMotionProperties::Vertex> SoftBodyVertices;
        JPH::Vec3 BodyPos;
        {
            BodyLockRead lock(physics_system->GetBodyLockInterface(), PhysicObject->GetBodyID());
            if (lock.Succeeded())
            {
                const Body& SoftBody = lock.GetBody();
                const SoftBodyMotionProperties* motionProperties = static_cast<const SoftBodyMotionProperties*>(SoftBody.GetMotionProperties());
                SoftBodyVertices = motionProperties->GetVertices();
                BodyPos = SoftBody.GetPosition();
            }
        }

#if SKIN == 1
        PhysicObject->SkinVertices(false, tempAllocator);
#endif
        int test = -1;

        JPH::Vec3 ToLocal = PhysicObject->CachePos;
        for (uint32 MeshIndex : PhysicObject->GetMeshIndices())
        {
            std::vector<::Vertex> Vertices = PhysicObject->GetMesh()->GetMeshData(MeshIndex)->Vertices;
            uint32 VertexNum = GetSize(Vertices);
            for (uint32 i = 0; i < VertexNum; ++i)
            {
                uint32 SoftBodyVertexIndex = PhysicObject->GetVertexIndex(::Vec3{ Vertices[i].Pos.x , Vertices[i].Pos.y, Vertices[i].Pos.z });
                Vertices[i].Pos.x = BodyPos.GetX() - ToLocal.GetX() + SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetX();
                Vertices[i].Pos.y = BodyPos.GetY() - ToLocal.GetY() + SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetY();
                Vertices[i].Pos.z = BodyPos.GetZ() - ToLocal.GetZ() + SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetZ();

                //Vertices[i].Pos.x = BodyPos.GetX() + SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetX();
                //Vertices[i].Pos.y = BodyPos.GetY() + SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetY();
                //Vertices[i].Pos.z = BodyPos.GetZ() + SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetZ();

                //Vertices[i].Pos.x = BodyPos.GetX();
                //Vertices[i].Pos.y = BodyPos.GetY();
                //Vertices[i].Pos.z = BodyPos.GetZ();

                if (test == -1 && SoftBodyVertices[SoftBodyVertexIndex].mInvMass == 0.f)
                {
                    test = SoftBodyVertexIndex;
                }
            }

            std::shared_ptr<MVertexBuffer> VertexBuffer = g_pRenderer->GetVertexBuffer(PhysicObject->GetPrimitiveComponent()->GetPrimitiveID(), MeshIndex);
            VertexBuffer->Update(Vertices.data());
        }

        if (PhysicObject->bTest)
        {
            //std::cout << "SoftBodyPos " << "X:" << BodyPos.GetX() << ", Y:" << BodyPos.GetY() << ", Z:" << BodyPos.GetZ() << std::endl;
            //std::cout << "ToLocal " << "X:" << ToLocal.GetX() << ", Y:" << ToLocal.GetY() << ", Z:" << ToLocal.GetZ() << std::endl;
            //std::cout << "ToLocal " << "X:" << SoftBodyVertices[test].mPosition.GetX() << ", Y:" << SoftBodyVertices[test].mPosition.GetY() << ", Z:" << SoftBodyVertices[test].mPosition.GetZ() << std::endl;
        }
        //std::cout << "SoftBodyPos " << "X:" << BodyPos.GetX() << ", Y:" << BodyPos.GetY() << ", Z:" << BodyPos.GetZ() << std::endl;

        PhysicObject->bTest = false;
    }
}

void MJoltPhysics::Release()
{
    MPhysicsEngine::Release();

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

void MJoltPhysicsObject::MoveTo(const ::Vec3& TargetPos)
{
    GetPhysicsSystem()->GetBodyInterface().MoveKinematic(BodyIDCache, RVec3Arg(TargetPos.x, TargetPos.y, TargetPos.z), Quat::sIdentity(), g_pMainGame->getDeltaTime());
}

void MJoltPhysicsObject::Remove()
{
    GetPhysicsSystem()->GetBodyInterface().RemoveBody(BodyIDCache);
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
    JPH::Vec3 JInPos = { InPos.x, InPos.y, InPos.z };

    Body& body = GetBody();
    if (body.IsSoftBody())
    {
        if (CachePos != JInPos)
        {
            GetPhysicsSystem()->GetBodyInterface().SetPosition(BodyIDCache, JInPos, EActivation::Activate);
            CachePos = JInPos;
            bTest = true;
        }
    }
    else
    {
        GetPhysicsSystem()->GetBodyInterface().SetPosition(BodyIDCache, JInPos, EActivation::Activate);
    }

    //std::cout << "SetPos Target: " << InPos.x << ", " << InPos.y << ", " << InPos.z << std::endl;
    //std::cout << "SetPos Current: " << 
}

void MJoltPhysicsObject::SetRotation(const ::Vec4& InRotation)
{
    JPH::Quat JInQuat = { InRotation.x, InRotation.y, InRotation.z,InRotation.w };
    
    Body& body = GetBody();
    if (body.IsSoftBody())
    {
        GetPhysicsSystem()->GetBodyInterface().SetRotation(BodyIDCache, JInQuat, EActivation::Activate);
    }
    else
    {

    }
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
    BodyLockWrite lock(GetPhysicsSystem()->GetBodyLockInterface(), GetBodyID());
    if (lock.Succeeded())
    {
        Body& Body = lock.GetBody();
        if (Body.GetMotionPropertiesUnchecked() == nullptr)
        {
            return;
        }
        
        JPH::Vec3 Arg;
        Arg.SetX(InForce.x * 10000.f);
        Arg.SetY(InForce.y * 10000.f);
        Arg.SetZ(InForce.z * 10000.f);
        Body.AddForce(Arg);
    }
}

void MJoltPhysicsObject::SetVelocity(const ::Vec3& InVelocity)
{
    JPH::Vec3 JPHVelociy = { InVelocity.x, InVelocity.y, InVelocity.z };

    PrevVeloc = JPHVelociy;
    Body& body = GetBody();
    if (body.IsSoftBody())
    {
        if (body.GetPosition() != JPHVelociy)
        {
            // InvMass 0인 점들에 속도를 설정하는 방법 ---------------------------------------------------------------------------------------
            SoftBodyMotionProperties* motionProperties = static_cast<SoftBodyMotionProperties*>(body.GetMotionProperties());
            uint32 Num = GetSize(motionProperties->GetVertices());
            for (uint32 i = 0; i < Num; ++i)
            {
                auto& Vtx = motionProperties->GetVertex(i);
                if (Vtx.mInvMass == 0.f)
                {
                    Vtx.mVelocity = JPHVelociy - body.GetPosition();
                }
            }
        }
    }
    else
    {
        GetPhysicsSystem()->GetBodyInterface().SetLinearVelocity(BodyIDCache, RVec3Arg(InVelocity.x, InVelocity.y, InVelocity.z));
    }
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

JPH::Body& MJoltPhysicsObject::GetBody()
{
    BodyLockWrite lock(GetPhysicsSystem()->GetBodyLockInterface(), BodyIDCache);
    return lock.GetBody();
}

uint32 MJoltPhysicsObject::GetVertexIndex(const ::Vec3& Pos)
{
    FVertexKey Key = { Pos.x, Pos.y, Pos.z };
    if (VertexIndex.find(Key) != VertexIndex.end())
    {
        return VertexIndex[Key];
    }

    return 0;
}

void MJoltPhysicsObject::SkinVertices(bool bHard, TempAllocator* Alloc)
{
    Body& body = GetBody();

    auto DynamicMeshComp = std::static_pointer_cast<DynamicMeshComponent>(GetPrimitiveComponent());
    Mat44& offset = body.GetCenterOfMassTransform().InversedRotationTranslation().ToMat44();
    //uint32 JointNum = DynamicMeshComp->GetDynamicMesh()->GetJointNum();
    uint32 JointNum = 1;
    uint32 JointIndex = DynamicMeshComp->GetDynamicMesh()->GetJointIndex(BoneName);
    static Array<Mat44> Pose(JointNum);

    //for (uint32 i = 0; i < JointNum; ++i)
    //{
        const Mat44* AnimPose = reinterpret_cast<const Mat44*>(&DynamicMeshComp->GetAnimMatrix(JointIndex));
        Pose[0] = offset * (*AnimPose);
    //}

    auto tf = body.GetCenterOfMassTransform();
    SoftBodyMotionProperties* mp = static_cast<SoftBodyMotionProperties*>(body.GetMotionProperties());
    mp->SetEnableSkinConstraints(true);
    mp->SkinVertices(tf, Pose.data(), JointNum, bHard, *Alloc);
}