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
#include "Jolt/Physics/Collision/Shape/EmptyShape.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/SoftBody/SoftBodyCreationSettings.h"
#include "Jolt/Physics/SoftBody/SoftBodyMotionProperties.h"
#include "Jolt/Physics/SoftBody/SoftBodyShape.h"
#include "Jolt/Physics/Constraints/FixedConstraint.h"
#include "Jolt/Physics/Character/Character.h"
#include "Jolt/Physics/Character/CharacterVirtual.h"
#include "Jolt/ObjectStream/ObjectStreamTextOut.h"
#include "Jolt/ObjectStream/ObjectStreamTextIn.h"


#include "MoonEngine.h"
#include "World.h"

#include "Renderer.h"
#include "Vertex.h"
#include "VertexBuffer.h"
#include "PrimitiveComponent.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"
#include "DynamicMeshComponent.h"
#include "Core/FileSystem.h"
#include <DirectXMath.h>

#include "Module/Physics/CapsuleComponent.h"
#include "Module/Physics/CapsuleBody.h"

using namespace JPH;
using namespace JPH::literals;
using namespace std;
using namespace DirectX;

constexpr char* BoneName = "bone001";

#define SKIN 0
#undef min
#undef max

#ifdef JPH_ENABLE_ASSERTS
    //static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
    //{
    //    // Print to the TTY
    //    cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << endl;

    //    // Breakpoint
    //    return true;
    //};
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
    static constexpr ObjectLayer CLOTH = 2;
    static constexpr ObjectLayer PHYSICS = 3;
    static constexpr ObjectLayer NUM_LAYERS = 4;
};

// 레이어간 충돌 여부를 설정
class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
    virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
    {
        switch (inObject1)
        {
        case Layers::NON_MOVING:
            return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING; // Non moving only collides with moving
        case Layers::MOVING:
            return inObject2 == Layers::NON_MOVING || inObject2 == Layers::MOVING; // Moving collides with everything
        case Layers::CLOTH:
            return inObject2 == Layers::PHYSICS;
        case Layers::PHYSICS:
            return inObject2 == Layers::CLOTH;
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
    static constexpr BroadPhaseLayer CLOTH(2);
    static constexpr BroadPhaseLayer PHYSICS(3);
    static constexpr uint NUM_LAYERS(4);
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
        mObjectToBroadPhase[Layers::CLOTH] = BroadPhaseLayers::CLOTH;
        mObjectToBroadPhase[Layers::PHYSICS] = BroadPhaseLayers::PHYSICS;
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
        case (BroadPhaseLayer::Type)BroadPhaseLayers::CLOTH:	    return "CLOTH";
        case (BroadPhaseLayer::Type)BroadPhaseLayers::PHYSICS:	    return "PHYSICS";
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
    virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1)
        {
        case Layers::NON_MOVING:
            return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING;
        case Layers::MOVING:
            return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING;
        case Layers::CLOTH:
            return inLayer2 == BroadPhaseLayers::PHYSICS;
        case Layers::PHYSICS:
            return inLayer2 == BroadPhaseLayers::CLOTH;
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
        //cout << "Contact validate callback" << endl;

        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void			OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
    {
        //cout << "A contact was added" << endl;
    }

    virtual void			OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) override
    {
        //cout << "A contact was persisted" << endl;
    }

    virtual void			OnContactRemoved(const SubShapeIDPair& inSubShapePair) override
    {
        //cout << "A contact was removed" << endl;
    }
};

// An example activation listener
class MyBodyActivationListener : public BodyActivationListener
{
public:
    virtual void		OnBodyActivated(const BodyID& inBodyID, uint64 inBodyUserData) override
    {
        //cout << "A body got activated" << endl;
    }

    virtual void		OnBodyDeactivated(const BodyID& inBodyID, uint64 inBodyUserData) override
    {
        //cout << "A body went to sleep" << endl;
    }
};

void MJoltPhysics::StartSimulate(MWorld* InWorld)
{
    MPhysicsEngine::StartSimulate(InWorld);

    for (auto& [Name, Actor] : InWorld->GetActors())
    {
        for (auto& [Name, Comp] : Actor->GetComponents())
        {
            std::shared_ptr<MMeshComponent>& MeshComp = Comp->CastToShared<MMeshComponent>();
            if (MeshComp == nullptr)
            {
                continue;
            }

            if (MeshComp->GetWorld() != InWorld)
            {
                continue;
            }

            std::shared_ptr<MMesh>& Mesh = MeshComp->GetMesh();
            if (Mesh == nullptr)
            {
                continue;
            }

            std::shared_ptr<MPhysics>& Physics = Mesh->GetPhysics();
            if (Physics == nullptr)
            {
                continue;
            }

            std::string Path = WStringToString(MFileSystem::AbsolutePath(Physics->GetAssetPath()));

            BodyInterface& bodyInterface = physics_system->GetBodyInterface();

            ShapeSettings* ShapeSetting = nullptr;
            std::stringstream ss;
            ObjectStreamTextIn StreamIn = JPH::ObjectStreamTextIn(ss);
            StreamIn.sReadObject(Path.c_str(), ShapeSetting);

            if (ShapeSetting == nullptr)
            {
                continue;
            }

            Ref<Shape> NewShape = ShapeSetting->Create().Get();
            JPH::Vec3 Pos = ToJPHPos(MeshComp->getWorldTranslation());
            JPH::Quat Rot = DXAngleToJPHQuat(MeshComp->getRotation());
            EMotionType MotionType = ConvertPhysicsType(MeshComp->GetPhysicsType());
            ObjectLayer Layer = (MotionType != EMotionType::Dynamic) ? Layers::NON_MOVING : Layers::MOVING;

            BodyCreationSettings BodyCreationSetting = BodyCreationSettings(NewShape, Pos, Rot, MotionType, Layer);
            //BodyCreationSetting.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
            BodyCreationSetting.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia;
            if (MotionType == EMotionType::Kinematic)
            {
                BodyCreationSetting.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
                BodyCreationSetting.mMassPropertiesOverride.ScaleToMass(1.f);
            }

            EActivation Activation = EActivation::DontActivate;
            if (MeshComp->IsPhysicsEnable() && bSimulating)
            {
                Activation = EActivation::Activate;
            }
            BodyID NewBodyID = bodyInterface.CreateAndAddBody(BodyCreationSetting, Activation);

            FBodyConstructData Data;
            Data.Mesh = Mesh;
            Data.PrimitiveComponent = MeshComp;

            std::shared_ptr<MBodyObject> NewPhysicsObject = std::make_shared<MBodyObject>(Data);
            NewPhysicsObject->SetBodyID(NewBodyID);
            NewPhysicsObject->SetScale(MeshComp->getScale());

            MeshComp->PhysicsObject = NewPhysicsObject;
            PhysicsObjects[MeshComp->GetPrimitiveID()].push_back(NewPhysicsObject);

            // ReadObject가 new를 이용해 Object를 생성하니, 삭제도 해줘야 함
            delete ShapeSetting;
        }
    }
}

JPH::Quat MJoltPhysics::DXQuatToJPHQuat(const::Vec4& InQuat)
{
    return JPH::Quat(-InQuat.x, -InQuat.y, InQuat.z, InQuat.w).Normalized(); // -y, -x, z, w 가 아닌지???
}

JPH::Quat MJoltPhysics::DXAngleToJPHQuat(const ::Vec3& InRot)
{
    ::Vec4 DXQuat = {};
    XMStoreFloat4(&DXQuat, XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&InRot)));
    return DXQuatToJPHQuat(DXQuat);
}

::Vec4 MJoltPhysics::JoltQuatToDXQuat(JPH::Quat InQuat)
{
    JPH::Vec3 Angle = InQuat.GetEulerAngles();

    JPH::Quat QuatX = JPH::Quat::sEulerAngles({ -Angle.GetX(), 0.f, 0.f });
    JPH::Quat QuatY = JPH::Quat::sEulerAngles({ 0.f, -Angle.GetY(), 0.f });
    JPH::Quat QuatZ = JPH::Quat::sEulerAngles({ 0.f, 0.f, Angle.GetZ() });

    JPH::Quat QxPrime = QuatY * QuatX * QuatY.Conjugated();
    JPH::Quat QyPrime = QuatY;
    QxPrime = QxPrime.Normalized();
    QyPrime = QyPrime.Normalized();

    JPH::Quat QuatYXZ = ((QuatZ * QxPrime) * QyPrime).Normalized();

    return { QuatYXZ.GetX(), QuatYXZ.GetY(), QuatYXZ.GetZ(), QuatYXZ.GetW() };
}

JPH::Vec3 MJoltPhysics::ToJPHPos(const::Vec3& InPos)
{
	return { InPos.x, InPos.y, -InPos.z };
}

JPH::Vec3 MJoltPhysics::ToJPHPos(const ::Vec4& InPos)
{
    return ToJPHPos({ InPos.x, InPos.y, InPos.z });
}

void MJoltPhysics::LoadTest()
{
    ConvexHullShapeSettings* Test = nullptr;

    std::stringstream ss;
    JPH::ObjectStreamTextIn StreamIn = JPH::ObjectStreamTextIn(ss);
    StreamIn.sReadObject("D:\\Git\\Moon\\JoltTest.physics", Test);

    Ref<Shape> NewShape = Test->Create().Get();
}

void MJoltPhysics::SaveTest(std::shared_ptr<MMesh> InMesh)
{
    if (InMesh == nullptr)
    {
        return;
    }

    BodyInterface& bodyInterface = physics_system->GetBodyInterface();

    const std::vector<::Vec3>& Vertices = InMesh->GetAllVertexPosition();
    const std::vector<uint32>& Indices = InMesh->GetMeshData(0).Indices;

    std::vector<JPH::Vec3> JPHVertices(Vertices.size());
    for (int i = 0; i < Vertices.size(); ++i)
    {
        JPHVertices[i] = ToJPHPos(Vertices[i]);
    }

    std::filesystem::path Path = MFileSystem::AbsolutePath(InMesh->GetAssetPath());
    Path.replace_extension("physics");

    // 저장 테스트
    std::stringstream ss;
    JPH::ObjectStreamTextOut streamOut = JPH::ObjectStreamTextOut(ss);
    streamOut.sWriteObject(Path.string().c_str(), JPH::ObjectStream::EStreamType::Text, MakeMeshShape(InMesh->CastToShared<StaticMesh>()));
    //streamOut.sWriteObject(Path.string().c_str(), JPH::ObjectStream::EStreamType::Text, ConvexHullShapeSettings(JPHVertices.data(), GetSize(JPHVertices)));
    
    std::shared_ptr<MPhysics> NewPhysics = std::make_shared<MPhysics>();
    NewPhysics->SetAssetPath(Path);

    InMesh->SetPhysics(NewPhysics);
}

JPH::ConvexHullShapeSettings MJoltPhysics::MakeConvexHull(FBodyConstructData& InData)
{
    BodyInterface& bodyInterface = physics_system->GetBodyInterface();

    const std::vector<::Vec3>& Vertices = InData.Mesh->GetAllVertexPosition();
    const std::vector<uint32>& Indices = InData.Mesh->GetMeshData(0).Indices;

    std::vector<JPH::Vec3> JPHVertices(Vertices.size());
    for (int i = 0; i < Vertices.size(); ++i)
    {
		JPHVertices[i] = ToJPHPos(Vertices[i]);
    }

    return ConvexHullShapeSettings(JPHVertices.data(), GetSize(JPHVertices));
}

JPH::MeshShapeSettings MJoltPhysics::MakeMeshShape(std::shared_ptr<StaticMesh> InMesh)
{
    JPH::VertexList vertexList;
    vertexList.resize(InMesh->GetAllVertexPosition().size());

    IndexedTriangleList triangleList; // 인덱스 3개로 이뤄진 삼감형들
    triangleList.reserve(vertexList.size() * 3);

    uint32 Offset = 0;
    for (uint32 MeshIndex = 0; MeshIndex < InMesh->GetMeshNum(); ++MeshIndex)
    {
        const auto& MeshData = InMesh->GetMeshData(MeshIndex);

        uint32 VertexNum = GetSize(MeshData.Vertices);
        for (uint32 VtxIndex = 0; VtxIndex < VertexNum; ++VtxIndex)
        {
            ToJPHPos(MeshData.Vertices[VtxIndex].Pos).StoreFloat3(&vertexList[Offset + VtxIndex]);
        }

        uint32 IndexLoopNum = GetSize(MeshData.Indices) / 3;
        for (uint32 i = 0; i < IndexLoopNum; ++i)
        {
            // TriangleList는 반드시 반시계 방향으로 넣어줘야 함 
            // https://jrouwe.github.io/JoltPhysics/class_mesh_shape_settings.html mIndexedTriangles 항목 참고
            triangleList.push_back(IndexedTriangle(Offset + MeshData.Indices[i * 3 + 2], Offset + MeshData.Indices[i * 3 + 1], Offset + MeshData.Indices[i * 3]));
        }

        Offset += VertexNum;
    }

    triangleList.shrink_to_fit();

    return MeshShapeSettings(vertexList, triangleList);
}

JPH::CapsuleShapeSettings MJoltPhysics::MakeCapsule(float InHalfHeight, float InRadius)
{
    return CapsuleShapeSettings(InHalfHeight, InRadius);
}

JPH::SphereShapeSettings MJoltPhysics::MakeSphere(float InRadius)
{
	return SphereShapeSettings(InRadius);
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

void MJoltPhysics::AddCloth(FBodyConstructData& InData, std::vector<FClothData>& ClothDatas, std::shared_ptr<MPhysicsObject>& OutPhysicsObject)
{
    BodyInterface& bodyInterface = physics_system->GetBodyInterface();

    SoftBodySharedSettings* NewSharedSettings = new SoftBodySharedSettings();

    // 중첩을 제거한 버텍스, 인덱스 쌍
    std::unordered_map<FVertexKey, uint32> VertexIndex;

    std::vector<uint32> MeshIndices;
    for (const FClothData& ClothData : ClothDatas)
    {
        MeshIndices.push_back(ClothData.MeshIndex);
        const FMeshData& MeshData = InData.Mesh->GetMeshData(ClothData.MeshIndex);
        const FJoint& Joint = InData.PrimitiveComponent->CastToShared<DynamicMeshComponent>()->GetJoint("bone001"); // 임시코드

        std::wstring Msg = TEXT("옷감 버텍스 수: ") + std::to_wstring(GetSize(MeshData.Vertices)) + TEXT(", 인덱스 수: ") + std::to_wstring(GetSize(MeshData.Indices));
        LOG(Msg);

        for (uint32 i = 0; i < GetSize(MeshData.Vertices); ++i)
        {
            const ::Vec4& VtxPos = MeshData.Vertices[i].Pos;
            FVertexKey VertexKey = { VtxPos.x, VtxPos.y, VtxPos.z };

            if (VertexIndex.find(VertexKey) != VertexIndex.end())
            {
                continue;
            }

            VertexIndex[VertexKey] = GetSize(NewSharedSettings->mVertices);

            JPH::Vec3 JoltVtxPos = ToJPHPos(VtxPos);
            JoltVtxPos -= ToJPHPos(Joint.Position);

            SoftBodySharedSettings::Vertex NewVertex;
            JoltVtxPos.StoreFloat3(&NewVertex.mPosition);
            NewVertex.mInvMass = VtxPos.y > 1.f ? 0.f : 1.f;
            NewSharedSettings->mVertices.push_back(NewVertex);
        }

        uint32 IndexLoopNum = GetSize(MeshData.Indices) / 3;
        for (uint32 i = 0; i < IndexLoopNum; ++i)
        {
            SoftBodySharedSettings::Face NewFace;

            for (uint32 j = 0; j < 3; ++j)
            {
                uint32 Index = MeshData.Indices[i * 3 + j];
                const ::Vec4& VtxPos = MeshData.Vertices[Index].Pos;

                FVertexKey VertexKey = { VtxPos.x, VtxPos.y, VtxPos.z };
                Index = VertexIndex[VertexKey];

                NewFace.mVertex[j] = Index;
            }

            NewSharedSettings->AddFace(NewFace);
        }
    }

#if SKIN == 1
    // 바인드 포즈 역행렬 ---------------------------------------------------------------------------------------------
    if (std::shared_ptr<DynamicMesh>& _DynamicMesh = InData.Mesh->CastToShared<DynamicMesh>())
    {
        const FJoint& Joint = _DynamicMesh->GetJoint("bone001");
        Mat4 MyMat = Joint._globalBindPoseInverseMatrix;

        // LH(X+Y+Z+) -> RH(X+Y+Z-)
        Mat4 Temp = MyMat;
        float s[4] = { 1.0f, 1.0f, -1.0f, 1.0f };
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                MyMat.m[i][j] = s[i] * Temp.m[i][j] * s[j];
            }
        }

        // Row Major -> Column Major
        XMStoreFloat4x4(&MyMat, XMMatrixTranspose(XMLoadFloat4x4(&MyMat)));

        Mat44 JoltMat = {
            Vec4Arg{MyMat._11, MyMat._12, MyMat._13, MyMat._14},
            Vec4Arg{MyMat._21, MyMat._22, MyMat._23, MyMat._24},
            Vec4Arg{MyMat._31, MyMat._32, MyMat._33, MyMat._34},
            Vec4Arg{MyMat._41, MyMat._42, MyMat._43, MyMat._44},
        };

        NewSharedSettings->mInvBindMatrices.emplace_back(0, JoltMat);
    }

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
    NewSharedSettings->mVertexRadius = 0.02f;
#endif

    // 바디 생성 ---------------------------------------------------------------------------------------------
    NewSharedSettings->Optimize();
    JPH::Vec3 Pos = JPH::Vec3::sZero();
    JPH::Quat Rot = JPH::Quat::sIdentity();

    SoftBodyCreationSettings ClothCreateSetting(NewSharedSettings, Pos, Rot, Layers::CLOTH);
    //ClothCreateSetting.mAllowSleeping = false;
    //ClothCreateSetting.mLinearDamping = 0.f;
    BodyID bodyId = bodyInterface.CreateAndAddSoftBody(ClothCreateSetting, EActivation::Activate);

    std::shared_ptr<MBodyObject> JoltPhysicsObject = std::make_shared<MBodyObject>(InData);
    JoltPhysicsObject->SetBodyID(bodyId);
    JoltPhysicsObject->SetMeshIndices(MeshIndices);
    JoltPhysicsObject->SetVertexIndices(VertexIndex);
    //JoltPhysicsObject->CachePos = InData.Pos;

    OutPhysicsObject = JoltPhysicsObject;
    SoftBodies.push_back(OutPhysicsObject);

    // 스키닝 적용 ---------------------------------------------------------------------------------------------
#if SKIN == 1
    JoltPhysicsObject->SkinVertices(true, tempAllocator);
#endif

    SoftBodyMotionProperties* mp = static_cast<SoftBodyMotionProperties*>(JoltPhysicsObject->GetBody().GetMotionProperties());
    mp->SetUpdatePosition(false);
}

void MJoltPhysics::AddCharacterPhyscics(std::shared_ptr<DynamicMeshComponent> InDynamicMeshComp, FBodyCapsuleData& InBodyCapsuleData)
{
    RefConst<Shape> shape = MakeCapsule(InBodyCapsuleData.GetHalfHeight(), InBodyCapsuleData.GetRadius()).Create().Get();

    FBodyConstructData BodyConstructData = {};
    BodyConstructData.PhysicsType = EPhysicsType::Kinematic;
    BodyConstructData.Mesh = InDynamicMeshComp->GetDynamicMesh();
    BodyConstructData.PrimitiveComponent = InDynamicMeshComp;
    BodyConstructData.Pos = InDynamicMeshComp->GetJointPosition(InBodyCapsuleData.AttachJointIndex);
    BodyConstructData.Rot = InDynamicMeshComp->GetJointQuaternion(InBodyCapsuleData.AttachJointIndex);

    std::shared_ptr<MCapsuleBody> NewCharacterBody = std::make_shared<MCapsuleBody>(BodyConstructData, InBodyCapsuleData);
    CreateBody(shape, BodyConstructData, NewCharacterBody, Layers::PHYSICS);

    PhysicsObjects[InDynamicMeshComp->GetPrimitiveID()].push_back(NewCharacterBody);
}

void MJoltPhysics::AddCharacterCollision(std::shared_ptr<MCollisionComponent> InComp, const FCapsuleData& InCapsuleData, std::shared_ptr<MPhysicsObject>& OutPhysicsObject)
{
    JPH::Vec3 Pos = ToJPHPos(InComp->getWorldTranslation());
    JPH::Quat RotQuat = DXAngleToJPHQuat(InComp->getRotation());

    RefConst<Shape> NewCapsuleShape = MakeCapsule(InCapsuleData.HalfHeight, InCapsuleData.Radius).Create().Get();

    //CharacterVirtualSettings NewCharacterVirtualSettings;
    //NewCharacterVirtualSettings.mShape = NewCapsuleShape;
    //CharacterVirtual* NewCharacterVirtual = new CharacterVirtual(&NewCharacterVirtualSettings, Pos, RotQuat, physics_system);

    CharacterSettings NewCharacterSettings;
    NewCharacterSettings.mLayer = Layers::MOVING;
    NewCharacterSettings.mShape = NewCapsuleShape;
    NewCharacterSettings.mMaxSlopeAngle = DegreesToRadians(45.0f);
    NewCharacterSettings.mFriction = 0.5f;
    NewCharacterSettings.mSupportingVolume = Plane(JPH::Vec3::sAxisY(), -0.3f);
    //Character* NewCharacter = new Character(&NewCharacterSettings, Pos, RotQuat, 0, physics_system);
    NewCharacter = new Character(&NewCharacterSettings, Pos, RotQuat, 0, physics_system);

    NewCharacter->AddToPhysicsSystem(EActivation::Activate);

    FBodyConstructData Data = {};
    std::shared_ptr<MBodyObject> JoltPhysicsObject = std::make_shared<MBodyObject>(Data);
    JoltPhysicsObject->SetBodyID(NewCharacter->GetBodyID());

    OutPhysicsObject = JoltPhysicsObject;
}

void MJoltPhysics::CreateBody(RefConst<Shape> InShape, const FBodyConstructData& InData, std::shared_ptr<MBodyObject> InBodyObject)
{
    if (InBodyObject == nullptr)
    {
        return;
    }

	BodyInterface& bodyInterface = physics_system->GetBodyInterface();
    JPH::Vec3 Pos = JPH::Vec3::sZero();
    JPH::Quat Rot = JPH::Quat::sIdentity();

	EMotionType MotionType = ConvertPhysicsType(InData.PhysicsType);
	JPH::ObjectLayer Layer = (MotionType == EMotionType::Static) ? Layers::NON_MOVING : Layers::MOVING;
	JPH::EActivation Activation = bSimulating ? EActivation::Activate : EActivation::DontActivate;

    BodyCreationSettings bodyCreationSettings(InShape.GetPtr(), Pos, Rot, MotionType, Layer);
    bodyCreationSettings.mMassPropertiesOverride.mMass = 0.f;
    bodyCreationSettings.mGravityFactor = 0.f;

	BodyID NewBodyID = bodyInterface.CreateAndAddBody(bodyCreationSettings, Activation);

	InBodyObject->SetBodyID(NewBodyID);
}


void MJoltPhysics::CreateBody(RefConst<Shape> InShape, const FBodyConstructData& InData, std::shared_ptr<MBodyObject> InBodyObject, JPH::ObjectLayer InLayer)
{
    if (InBodyObject == nullptr)
    {
        return;
    }

    BodyInterface& bodyInterface = physics_system->GetBodyInterface();
    JPH::Vec3 Pos = JPH::Vec3::sZero();
    JPH::Quat Rot = JPH::Quat::sIdentity();

    EMotionType MotionType = ConvertPhysicsType(InData.PhysicsType);
    JPH::EActivation Activation = bSimulating ? EActivation::Activate : EActivation::DontActivate;

    BodyCreationSettings bodyCreationSettings(InShape.GetPtr(), Pos, Rot, MotionType, InLayer);
    bodyCreationSettings.mMassPropertiesOverride.mMass = 0.f;
    bodyCreationSettings.mGravityFactor = 0.f;

    BodyID NewBodyID = bodyInterface.CreateAndAddBody(bodyCreationSettings, Activation);

    InBodyObject->SetBodyID(NewBodyID);
}


void MJoltPhysics::Constraint(std::shared_ptr<MPhysicsObject>& Lhs, std::shared_ptr<MPhysicsObject>& Rhs)
{
    std::shared_ptr<MBodyObject> lhs = std::static_pointer_cast<MBodyObject>(Lhs);
    std::shared_ptr<MBodyObject> rhs = std::static_pointer_cast<MBodyObject>(Rhs);

    FixedConstraintSettings* ConstraintSetting = new FixedConstraintSettings();
    physics_system->AddConstraint(ConstraintSetting->Create(lhs->GetBody(), rhs->GetBody()));
}

bool MJoltPhysics::Initialize()
{
    Super::Initialize();

    RegisterDefaultAllocator();

    Trace = TraceImpl;
    //JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

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

    return true;
}

void MJoltPhysics::Update()
{
    float DeltaTime = GetMainWorld()->getDeltaTime();
	static float TotalTime = 0.f;
	TotalTime += DeltaTime;

    physics_system->Update(DeltaTime, 2, tempAllocator, jobSystem);

    // 캐릭터 바디 업데이트
    for (auto& [PrimitiveID, ComponentPhyscisObjects] : PhysicsObjects)
    {
        for (auto& PhysicsObject : ComponentPhyscisObjects)
        {
            PhysicsObject->Update(DeltaTime);
        }
    }

    // 클로딩
    for (auto& SoftBodyObject : SoftBodies)
    {
        std::shared_ptr<MBodyObject> PhysicObject = std::static_pointer_cast<MBodyObject>(SoftBodyObject.lock());

        if (PhysicObject == nullptr)
        {
            continue;
        }

        JPH::Vec3 JoltJointPos = { 0.f, 0.f, 0.f };
        JPH::Quat JoltJointQuat = JPH::Quat::sIdentity();
        Array<SoftBodyMotionProperties::Vertex> SoftBodyVertices;
        {
            BodyLockWrite lock(physics_system->GetBodyLockInterface(), PhysicObject->GetBodyID());
            if (lock.Succeeded())
            {
                Body& SoftBody = lock.GetBody();
                const SoftBodyMotionProperties* motionProperties = static_cast<const SoftBodyMotionProperties*>(SoftBody.GetMotionProperties());
                SoftBodyVertices = motionProperties->GetVertices();
            }
        }

        if (std::shared_ptr<DynamicMeshComponent> DynamicMeshComp = PhysicObject->GetPrimitiveComponent()->CastToShared<DynamicMeshComponent>())
        {
            const FJoint Joint = DynamicMeshComp->GetJoint("bone001");

            ::Vec3 DXJointPos = DynamicMeshComp->GetJointPosition("bone001");
            ::Vec4 DXJointQuat = DynamicMeshComp->GetJointQuaternion("bone001");

            JoltJointPos = ToJPHPos(DXJointPos);
            JoltJointQuat = DXQuatToJPHQuat(DXJointQuat);

            //getRenderer()->DrawCapsule(GetMainWorld().get(), , ::Vec3{ JoltJointPos.GetX(), JoltJointPos.GetY(), -JoltJointPos.GetZ() }, VEC3ZERO);
            //getRenderer()->DrawCoordinate(GetMainWorld().get(), ::Vec3{ JoltJointPos.GetX(), JoltJointPos.GetY(), -JoltJointPos.GetZ() }, VEC3ZERO);
        }

        physics_system->GetBodyInterface().SetPositionAndRotation(PhysicObject->GetBodyID(), JoltJointPos, JoltJointQuat, EActivation::Activate);
    }

    // SoftBody의 정점위치 갱신
    for (auto& SoftBodyObject : SoftBodies)
    {
        std::shared_ptr<MBodyObject> PhysicObject = std::static_pointer_cast<MBodyObject>(SoftBodyObject.lock());

        if (PhysicObject == nullptr)
        {
            continue;
        }

        Array<SoftBodyMotionProperties::Vertex> SoftBodyVertices;
        JPH::Vec3 BodyPos = {};
		JPH::Quat BodyRot = {};
        {
            BodyLockRead lock(physics_system->GetBodyLockInterface(), PhysicObject->GetBodyID());
            if (lock.Succeeded())
            {
                const Body& SoftBody = lock.GetBody();
                const SoftBodyMotionProperties* motionProperties = static_cast<const SoftBodyMotionProperties*>(SoftBody.GetMotionProperties());
                SoftBodyVertices = motionProperties->GetVertices();

				BodyPos = SoftBody.GetPosition();
				BodyRot = SoftBody.GetRotation();
            }
        }

        ::Vec4 DxBodyQuat = JoltQuatToDXQuat(BodyRot);
        ::Vec3 DXPos = { BodyPos.GetX(), BodyPos.GetY(), -BodyPos.GetZ() };

        //getRenderer()->DrawCoordinate(GetMainWorld().get(), DXPos, DxBodyQuat);
        //getRenderer()->DrawCapsule(GetMainWorld().get(), 0.02f, 0.02f, DXPos, DxBodyQuat);

        const FJoint& Joint = PhysicObject->GetPrimitiveComponent()->CastToShared<DynamicMeshComponent>()->GetJoint("bone001");
        for (uint32 MeshIndex : PhysicObject->GetMeshIndices())
        {
            std::vector<::Vertex> Vertices = PhysicObject->GetMesh()->GetMeshData(MeshIndex).Vertices;
            uint32 VertexNum = GetSize(Vertices);
            for (uint32 i = 0; i < VertexNum; ++i)
            {
                uint32 SoftBodyVertexIndex = PhysicObject->GetVertexIndex(::Vec3{ Vertices[i].Pos.x , Vertices[i].Pos.y, Vertices[i].Pos.z });

				Vertices[i].Pos.x = SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetX() + Joint.Position.x;
				Vertices[i].Pos.y = SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetY() + Joint.Position.y;
				Vertices[i].Pos.z = -(SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetZ() + Joint.Position.z);
            }

            FBufferContainer BufferContainer = {};
            getGraphicDevice()->GetPrivateBuffers(BufferContainer, PhysicObject->GetPrimitiveComponent()->GetPrimitiveID());

            if (std::shared_ptr<MVertexBuffer> VertexBuffer = BufferContainer.VertexBuffers[MeshIndex])
            {
                VertexBuffer->Update(Vertices.data());
            }
        }
    }

    if (NewCharacter)
    {
        NewCharacter->PostSimulation(DeltaTime);
    }
}

void MJoltPhysics::Render()
{
    Super::Render();
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

MBodyObject::MBodyObject(const FBodyConstructData& InData)
    : MPhysicsObject(InData.PrimitiveComponent, InData.Mesh)
{

}

MBodyObject::~MBodyObject()
{
    GetPhysicsSystem()->GetBodyInterface().RemoveBody(BodyIDCache);
}

bool MBodyObject::IsSimulating()
{
    return GetPhysicsSystem()->GetBodyInterface().IsActive(BodyIDCache);
}

void MBodyObject::SetSimulate(bool bEnable)
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

void MBodyObject::SetMass(float InMass)
{
    //GetPhysicsSystem()->GetBodyInterface().
}

void MBodyObject::SetPos(const ::Vec3& InPos)
{
    JPH::Vec3 JInPos = MJoltPhysics::ToJPHPos(InPos);

    Body& body = GetBody();
    GetPhysicsSystem()->GetBodyInterface().SetPosition(BodyIDCache, JInPos, EActivation::Activate);
}

void MBodyObject::SetRotation(const ::Vec4& InRotation)
{
	JPH::Quat JInQuat = MJoltPhysics::DXQuatToJPHQuat(InRotation);

    Body& body = GetBody();
    GetPhysicsSystem()->GetBodyInterface().SetRotation(BodyIDCache, JInQuat, EActivation::Activate);
}

void MBodyObject::SetRotation(const ::Vec3& InRotation)
{
    ::Vec4 RotQuat = {};
    XMStoreFloat4(&RotQuat, XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&InRotation)));
    SetRotation(RotQuat);
}

void MBodyObject::SetScale(const ::Vec3& InScale)
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

void MBodyObject::SetGravity(bool bGravity)
{

}

void MBodyObject::AddForce(const ::Vec3& InForce)
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

void MBodyObject::SetVelocity(const ::Vec3& InVelocity)
{
    JPH::Vec3 JPHVelociy = { InVelocity.x, InVelocity.y, -InVelocity.z };

    PrevVeloc = JPHVelociy;
    Body& body = GetBody();

    GetPhysicsSystem()->GetBodyInterface().SetLinearVelocity(BodyIDCache, JPHVelociy);
}

void MBodyObject::SetAngularVelocity(const ::Vec3& InVelocity)
{

}

::Vec3 MBodyObject::GetPhysicsPos()
{
    JPH::Vec3 OutPos = GetPhysicsSystem()->GetBodyInterface().GetPosition(BodyIDCache);
    return { OutPos.GetX(), OutPos.GetY(), -OutPos.GetZ() };
}

::Vec3 MBodyObject::GetPhysicsRotation()
{
    ::Vec4 RotQuat = MJoltPhysics::JoltQuatToDXQuat(GetPhysicsSystem()->GetBodyInterface().GetRotation(BodyIDCache));
    ::Vec3 Rot = {};
    DXQuaternionToEuler(RotQuat, Rot.x, Rot.y, Rot.z);
    return Rot;
}

JPH::Body& MBodyObject::GetBody()
{
    BodyLockWrite lock(GetPhysicsSystem()->GetBodyLockInterface(), BodyIDCache);
    return lock.GetBody();
}

uint32 MBodyObject::GetVertexIndex(const ::Vec3& Pos)
{
    FVertexKey Key = { Pos.x, Pos.y, Pos.z };
    if (VertexIndex.find(Key) != VertexIndex.end())
    {
        return VertexIndex[Key];
    }

    return 0;
}

void MBodyObject::SkinVertices(bool bHard, TempAllocator* Alloc)
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