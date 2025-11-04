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

::Vec3 ConvertDirectXToJoltRotation(const ::Vec3& directXRot)
{
    XMVECTOR XMQuat = XMQuaternionRotationRollPitchYaw(-directXRot.x, directXRot.y, directXRot.z);

    return {};
}

// Rotation around X axis (right-handed)
inline XMMATRIX RH_RotationX(float angleRad)
{
    float c = cosf(angleRad);
    float s = sinf(angleRad);

    // Rx (row-major visual):
    // [1   0    0   0]
    // [0   c   -s   0]
    // [0   s    c   0]
    // [0   0    0   1]

    XMVECTOR r0 = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR r1 = XMVectorSet(0.0f, c, -s, 0.0f);
    XMVECTOR r2 = XMVectorSet(0.0f, s, c, 0.0f);
    XMVECTOR r3 = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    XMMATRIX M;
    M.r[0] = r0;
    M.r[1] = r1;
    M.r[2] = r2;
    M.r[3] = r3;
    return M;
}

// Rotation around Y axis (right-handed)
inline XMMATRIX RH_RotationY(float angleRad)
{
    float c = cosf(angleRad);
    float s = sinf(angleRad);

    // Ry (row-major):
    // [ c   0   s  0]
    // [ 0   1   0  0]
    // [-s   0   c  0]
    // [ 0   0   0  1]

    XMVECTOR r0 = XMVectorSet(c, 0.0f, s, 0.0f);
    XMVECTOR r1 = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR r2 = XMVectorSet(-s, 0.0f, c, 0.0f);
    XMVECTOR r3 = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    XMMATRIX M;
    M.r[0] = r0;
    M.r[1] = r1;
    M.r[2] = r2;
    M.r[3] = r3;
    return M;
}

// Rotation around Z axis (right-handed)
inline XMMATRIX RH_RotationZ(float angleRad)
{
    float c = cosf(angleRad);
    float s = sinf(angleRad);

    // Rz (row-major):
    // [ c  -s  0  0]
    // [ s   c  0  0]
    // [ 0   0  1  0]
    // [ 0   0  0  1]

    XMVECTOR r0 = XMVectorSet(c, -s, 0.0f, 0.0f);
    XMVECTOR r1 = XMVectorSet(s, c, 0.0f, 0.0f);
    XMVECTOR r2 = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR r3 = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    XMMATRIX M;
    M.r[0] = r0;
    M.r[1] = r1;
    M.r[2] = r2;
    M.r[3] = r3;
    return M;
}

XMVECTOR MakeQuat(FXMMATRIX M)
{
    const float m00 = XMVectorGetX(M.r[0]);
    const float m01 = XMVectorGetY(M.r[0]);
    const float m02 = XMVectorGetZ(M.r[0]);

    const float m10 = XMVectorGetX(M.r[1]);
    const float m11 = XMVectorGetY(M.r[1]);
    const float m12 = XMVectorGetZ(M.r[1]);

    const float m20 = XMVectorGetX(M.r[2]);
    const float m21 = XMVectorGetY(M.r[2]);
    const float m22 = XMVectorGetZ(M.r[2]);

    float trace = m00 + m11 + m22;
    float qw, qx, qy, qz;

    if (trace > 0.0f) {
        float S = sqrtf(trace + 1.0f) * 2.0f; // S = 4*qw
        qw = 0.25f * S;
        qx = (m21 - m12) / S;
        qy = (m02 - m20) / S;
        qz = (m10 - m01) / S;
    }
    else {
        if (m00 > m11 && m00 > m22) {
            float S = sqrtf(1.0f + m00 - m11 - m22) * 2.0f; // S = 4*qx
            qw = (m21 - m12) / S;
            qx = 0.25f * S;
            qy = (m01 + m10) / S;
            qz = (m02 + m20) / S;
        }
        else if (m11 > m22) {
            float S = sqrtf(1.0f + m11 - m00 - m22) * 2.0f; // S = 4*qy
            qw = (m02 - m20) / S;
            qx = (m01 + m10) / S;
            qy = 0.25f * S;
            qz = (m12 + m21) / S;
        }
        else {
            float S = sqrtf(1.0f + m22 - m00 - m11) * 2.0f; // S = 4*qz
            qw = (m10 - m01) / S;
            qx = (m02 + m20) / S;
            qy = (m12 + m21) / S;
            qz = 0.25f * S;
        }
    }

    XMVECTOR q = XMVectorSet(qx, qy, qz, qw);
    return q;
}

void MJoltPhysics::StartSimulate()
{
    MPhysicsEngine::StartSimulate();

    for (auto WeakMeshComp : MeshComponents)
    {
        auto MeshComp = WeakMeshComp.lock();

        std::shared_ptr<StaticMesh>& Mesh = MeshComp->GetMesh();
        if (Mesh == nullptr)
        {
            continue;
        }

        std::shared_ptr<MPhysics>& Physics = Mesh->GetPhysics();
        if (Physics == nullptr)
        {
            continue;
        }

        std::string Path = WStringToString(MFIleSystem::AbsolutePath(Physics->GetAssetPath()));

        BodyInterface& bodyInterface = physics_system->GetBodyInterface();

        ShapeSettings* ShapeSetting = nullptr;
        std::stringstream ss;
        ObjectStreamTextIn StreamIn = JPH::ObjectStreamTextIn(ss);
        StreamIn.sReadObject(Path.c_str(), ShapeSetting);

        ::Vec3 CompPos = MeshComp->getWorldTranslation();
        ::Vec3 CompRot = MeshComp->getRotation();

        Ref<Shape> NewShape = ShapeSetting->Create().Get();
        JPH::Vec3 Pos = ToJPHPos(CompPos);
        JPH::Quat Rot = DXAngleToJPHQuat(CompRot);
        EMotionType MotionType = ConvertPhysicsType(MeshComp->GetPhysicsType());
        ObjectLayer Layer = (MotionType == EMotionType::Static) ? Layers::NON_MOVING : Layers::MOVING;

        BodyCreationSettings BodyCreationSetting = BodyCreationSettings(NewShape, Pos, Rot, MotionType, Layer);

        if (MeshShapeSettings* MeshShapeSetting = DynamicCast<MeshShapeSettings>(ShapeSetting))
        {
            BodyCreationSetting.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
            BodyCreationSetting.mMassPropertiesOverride.mMass = 1.f;
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
        ::Vec3 CompScale = MeshComp->getScale();
        NewPhysicsObject->SetScale(CompScale);

        MeshComp->PhysicsObject = NewPhysicsObject;

        // ReadObject가 new를 이용해 Object를 생성하니, 삭제도 해줘야 함
        delete ShapeSetting;
    }
}

JPH::Quat MJoltPhysics::DXQuatToJPHQuat(const::Vec4& InQuat)
{
    ::Vec3 Angle = {};
    DXQuaternionToEuler(InQuat, Angle.x, Angle.y, Angle.z);
    return DXAngleToJPHQuat(Angle);
}

Quat FromAxisAngle(float ax, float ay, float az, float angle) {
    // axis must be normalized for correct result; here we assume axis is unit or single-axis.
    float s = std::sin(angle * 0.5f);
    float c = std::cos(angle * 0.5f);
    return Quat(ax * s, ay * s, az * s, c).Normalized();
}


JPH::Quat MJoltPhysics::DXAngleToJPHQuat(const ::Vec3& InRot)
{
    
    // 오른손 좌표계 xyz 회전 행렬. x,y 각도의 -처리는 행렬 계산 쪽에서 해줌.
    //::Vec3 RhRot = { InRot.x, InRot.y - PI / 2.f, InRot.z };
    //XMMATRIX XMRotMat = RH_RotationZ(RhRot.z) * RH_RotationY(RhRot.y) * RH_RotationX(RhRot.x);

    //::Vec4 FQuat = {};
    //XMVECTOR XMQuat = MakeQuat(XMRotMat);
    //XMStoreFloat4(&FQuat, XMQuat);

    //// z축 반전
    //JPH::Quat JoltQuat = { -FQuat.x, -FQuat.y, FQuat.z, FQuat.w };
    //return JoltQuat.Normalized();
    

    ::Vec3 RhRot = { -InRot.x, -InRot.y, InRot.z };
    Quat qx = FromAxisAngle(1.0f, 0.0f, 0.0f, RhRot.x);
    Quat qy = FromAxisAngle(0.0f, 1.0f, 0.0f, RhRot.y);
    Quat qz = FromAxisAngle(0.0f, 0.0f, 1.0f, RhRot.z);

    Quat OutQuat = qz * qy * qx;
    return OutQuat.Normalized();
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

void MJoltPhysics::SaveTest(std::shared_ptr<StaticMesh> InMesh)
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
    BodyInterface& bodyInterface = physics_system->GetBodyInterface();

    const std::vector<::Vec3>& Vertices = InMesh->GetAllVertexPosition();
    const std::vector<uint32>& Indices = InMesh->GetMeshData(0).Indices;

    JPH::VertexList vertexList;
	vertexList.resize(Vertices.size());
    for (int i = 0; i < Vertices.size(); ++i)
    {
		ToJPHPos(Vertices[i]).StoreFloat3(&vertexList[i]);
    }

    IndexedTriangleList triangleList;
    uint32 IndexLoopNum = GetSize(Indices) / 3;
    for (uint32 i = 0; i < IndexLoopNum; ++i)
    {
        triangleList.push_back(IndexedTriangle(Indices[i * 3], Indices[i * 3 + 1], Indices[i * 3 + 2]));
    }

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
        
        for (uint32 i = 0; i < GetSize(MeshData.Vertices); ++i)
        {
            const ::Vec4& VtxPos = MeshData.Vertices[i].Pos;
            FVertexKey VertexKey = { VtxPos.x, VtxPos.y, VtxPos.z };

            if (VertexIndex.find(VertexKey) != VertexIndex.end())
            {
                continue;
            }

            VertexIndex[VertexKey] = GetSize(NewSharedSettings->mVertices);

            const JPH::Vec3 JoltVtxPos = ToJPHPos(VtxPos);

            SoftBodySharedSettings::Vertex NewVertex;
            JoltVtxPos.StoreFloat3(&NewVertex.mPosition);
            NewVertex.mInvMass = JoltVtxPos.GetY() > 1.f ? 0.f : 1.f;
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
    if (std::shared_ptr<DynamicMesh>& _DynamicMesh = InData.Mesh->CastTo<DynamicMesh>())
    {
        //auto& Joints = _DynamicMesh->GetJoints();
        //Mat4 MyMat = Joints[_DynamicMesh->GetJointIndex("bone001")]._globalBindPoseInverseMatrix;

        //XMMATRIX XMMat = XMLoadFloat4x4(&MyMat) * XMMatrixScaling(1.f / 2.54f, 1.f / 2.54f, 1.f / 2.54f);
        //XMStoreFloat4x4(&MyMat, XMMat);

        // z축 반전
        //Mat4 Temp = MyMat;
        //float s[4] = { 1.0f, 1.0f, -1.0f, 1.0f };
        //for (int i = 0; i < 4; ++i) {
        //    for (int j = 0; j < 4; ++j) {
        //        MyMat.m[i][j] = s[i] * Temp.m[i][j] * s[j];
        //    }
        //}

        //XMStoreFloat4x4(&MyMat, XMMatrixTranspose(XMLoadFloat4x4(&MyMat)));

        //Mat44 mat2 = {
        //    Vec4Arg{MyMat._11, MyMat._12, MyMat._13, MyMat._14},
        //    Vec4Arg{MyMat._21, MyMat._22, MyMat._23, MyMat._24},
        //    Vec4Arg{MyMat._31, MyMat._32, MyMat._33, MyMat._34},
        //    Vec4Arg{MyMat._41, MyMat._42, MyMat._43, MyMat._44},
        //};

        //NewSharedSettings->mInvBindMatrices.emplace_back(0, mat2);
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
#endif

    // 바디 생성 ---------------------------------------------------------------------------------------------
    NewSharedSettings->Optimize();
	JPH::Vec3 Pos = ToJPHPos(InData.Pos);
	JPH::Quat Rot = DXQuatToJPHQuat(InData.Rot);

    SoftBodyCreationSettings ClothCreateSetting(NewSharedSettings, Pos, Rot, Layers::MOVING);
    ClothCreateSetting.mAllowSleeping = false;
    ClothCreateSetting.mLinearDamping = 0.f;
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

void MJoltPhysics::AddCharacterBody(std::shared_ptr<DynamicMeshComponent> InDynamicMeshComp, FBodyCapsuleData& InBodyCapsuleData)
{
    //RefConst<Shape> shape = MakeCapsule(5.f, 0.2f).Create().Get();
    //InBodyCapsuleData.HalfHeight = 5.f;
    //InBodyCapsuleData.Radius = 0.2f;

    RefConst<Shape> shape = MakeCapsule(0.15f, 0.12f).Create().Get();
    InBodyCapsuleData.CapsuleData.HalfHeight = 0.15f;
    InBodyCapsuleData.CapsuleData.Radius = 0.12f;

    //RefConst<Shape> shape = MakeCapsule(0.15f, 0.08f).Create().Get();
    //InBodyCapsuleData.HalfHeight = 0.15f;
    //InBodyCapsuleData.Radius = 0.08f;

	//RefConst<Shape> shape = MakeSphere(0.2f).Create().Get();

    FBodyConstructData BodyConstructData = {};
    BodyConstructData.PhysicsType = EPhysicsType::Kinematic;
    BodyConstructData.Mesh = InDynamicMeshComp->GetDynamicMesh();
    BodyConstructData.PrimitiveComponent = InDynamicMeshComp;
    BodyConstructData.Pos = InDynamicMeshComp->GetJointPosition(InBodyCapsuleData.AttachJointIndex);
    BodyConstructData.Rot = InDynamicMeshComp->GetJointQuaternion(InBodyCapsuleData.AttachJointIndex);

    std::shared_ptr<MCapsuleBody> NewCharacterBody = std::make_shared<MCapsuleBody>(BodyConstructData, InBodyCapsuleData);
    CreateBody(shape, BodyConstructData, NewCharacterBody);

    CapsuleBodies.push_back(NewCharacterBody);
}

void MJoltPhysics::CreateBody(RefConst<Shape> InShape, const FBodyConstructData& InData, std::shared_ptr<MBodyObject> InBodyObject)
{
    if (InBodyObject == nullptr)
    {
        return;
    }

	BodyInterface& bodyInterface = physics_system->GetBodyInterface();

	JPH::Vec3 Pos = ToJPHPos(InData.Pos);
    JPH::Quat Rot = DXQuatToJPHQuat(InData.Rot);

	EMotionType MotionType = ConvertPhysicsType(InData.PhysicsType);
	JPH::ObjectLayer Layer = (MotionType == EMotionType::Static) ? Layers::NON_MOVING : Layers::MOVING;
	JPH::EActivation Activation = bSimulating ? EActivation::Activate : EActivation::DontActivate;

    BodyCreationSettings bodyCreationSettings(InShape.GetPtr(), Pos, Rot, MotionType, Layer);
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

    return true;
}

void MJoltPhysics::Update()
{
    float DeltaTime = GetMainWorld()->getDeltaTime();
	static float TotalTime = 0.f;
	TotalTime += DeltaTime;

    physics_system->Update(DeltaTime, 1, tempAllocator, jobSystem);

    // 캐릭터 바디 업데이트
    for (auto& CharacterBody : CapsuleBodies)
    {
        CharacterBody->Update(DeltaTime);
    }

    // 클로딩
    for (auto& SoftBodyObject : SoftBodies)
    {
        std::shared_ptr<MBodyObject> PhysicObject = std::static_pointer_cast<MBodyObject>(SoftBodyObject.lock());

        if (PhysicObject == nullptr)
        {
            continue;
        }

        JPH::Vec3 JoltPos = {};
        JPH::Quat JoltQuat = JPH::Quat::sIdentity();
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

        if (std::shared_ptr<DynamicMeshComponent> DynamicMeshComp = PhysicObject->GetPrimitiveComponent()->CastTo<DynamicMeshComponent>())
        {
            ::Vec3 JointPos = {};   
            ::Vec3 JointAngle = {};

            XMVECTOR XMBase =  XMLoadFloat3(&DynamicMeshComp->GetDynamicMesh()->GetJoint("bone001").Position);
            ::Vec3 Base = {};
            XMStoreFloat3(&Base, XMBase);

            ::Vec3 CurrentPos = DynamicMeshComp->GetJointPosition("bone014");
            CurrentPos.x /= 2.54f;
            CurrentPos.y /= 2.54f;
            CurrentPos.z /= 2.54f;
            CurrentPos.y -= 1.f;

            //::Vec3 CurrentPos = DynamicMeshComp->GetJointPosition("bone4094");
            JointPos = CurrentPos;

            //JointPos.y = -0.15f;
            //XMStoreFloat3(&JointPos, XMLoadFloat3(&DynamicMeshComp->getWorldTranslation()) + XMLoadFloat3(&CurrentPos) - XMBase);
            //XMStoreFloat3(&JointPos, XMLoadFloat3(&DynamicMeshComp->getWorldTranslation()));
            JoltPos = ToJPHPos(JointPos);

            ::Vec4 JointQuat = DynamicMeshComp->GetJointQuaternion("bone014");
            JoltQuat = DXQuatToJPHQuat(DynamicMeshComp->GetJointQuaternion("bone014"));
            JoltQuat.SetX(0.f);
            JoltQuat.SetZ(0.f);
            JoltQuat = JoltQuat.Normalized();

            std::cout << "XM Pos: " << JointPos << std::endl;
            //std::cout << "XM Angle: " << ToDegree(JointAngle.x) << ", " << ToDegree(JointAngle.y) << ", " << ToDegree(JointAngle.z) << std::endl;

            //JPH::Quat JoltBodyRot = PhysicObject->GetBody().GetRotation();
            //JPH::Vec3 JoltBodyAngle = JoltBodyRot.GetEulerAngles();
            //std::cout << "Body Angle: " << ToDegree(JoltBodyAngle.GetX()) << ", " << ToDegree(JoltBodyAngle.GetY()) << ", " << ToDegree(JoltBodyAngle.GetZ()) << std::endl;

            //JPH::Vec3 tt = JoltQuat.GetEulerAngles();
            //std::cout << "New Body Angle: " << ToDegree(tt.GetX()) << ", " << ToDegree(tt.GetY()) << ", " << ToDegree(tt.GetZ()) << std::endl;
        }

        physics_system->GetBodyInterface().SetPositionAndRotation(PhysicObject->GetBodyID(), JoltPos, JoltQuat, EActivation::Activate);
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
		//std::cout << "SoftBody Pos: " << BodyPos.GetX() << ", " << BodyPos.GetY() << ", " << BodyPos.GetZ() << std::endl;
		//std::cout << "SoftBody Rot: " << BodyRot.GetX() << ", " << BodyRot.GetY() << ", " << BodyRot.GetZ() << ", " << BodyRot.GetZ() << std::endl;

        for (uint32 MeshIndex : PhysicObject->GetMeshIndices())
        {
            std::vector<::Vertex> Vertices = PhysicObject->GetMesh()->GetMeshData(MeshIndex).Vertices;
            uint32 VertexNum = GetSize(Vertices);
            for (uint32 i = 0; i < VertexNum; ++i)
            {
                uint32 SoftBodyVertexIndex = PhysicObject->GetVertexIndex(::Vec3{ Vertices[i].Pos.x , Vertices[i].Pos.y, Vertices[i].Pos.z });

				Vertices[i].Pos.x = SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetX();
				Vertices[i].Pos.y = SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetY();
				Vertices[i].Pos.z = -SoftBodyVertices[SoftBodyVertexIndex].mPosition.GetZ();
            }

            FBufferContainer BufferContainer = {};
            getGraphicDevice()->GetPrivateBuffers(BufferContainer, PhysicObject->GetPrimitiveComponent()->GetPrimitiveID());

            if (std::shared_ptr<MVertexBuffer> VertexBuffer = BufferContainer.VertexBuffers[MeshIndex])
            {
                VertexBuffer->Update(Vertices.data());
            }
        }
    }   
}

void MJoltPhysics::Render()
{
    Super::Render();

    if (getRenderer() && getRenderer()->bDrawCollision)
    {
        for (auto& CapsuleBody : CapsuleBodies)
        {
            CapsuleBody->Render();
        }
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

MBodyObject::MBodyObject(const FBodyConstructData& InData)
    : MPhysicsObject(InData.PrimitiveComponent, InData.Mesh)
{

}

void MBodyObject::MoveTo(const ::Vec3& TargetPos)
{
    //GetPhysicsSystem()->GetBodyInterface().MoveKinematic(BodyIDCache, RVec3Arg(TargetPos.x, TargetPos.y, TargetPos.z), Quat::sIdentity(), g_World->getDeltaTime());
}

void MBodyObject::Remove()
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

}

void MBodyObject::SetPos(const ::Vec3& InPos)
{
    JPH::Vec3 JInPos = MJoltPhysics::ToJPHPos(InPos);

    Body& body = GetBody();
    if (body.IsSoftBody())
    {
		//if (CachePos != JInPos)
		//{
		//	GetPhysicsSystem()->GetBodyInterface().SetPosition(BodyIDCache, JInPos, EActivation::Activate);
		//	CachePos = JInPos;
		//}

		GetPhysicsSystem()->GetBodyInterface().SetPosition(BodyIDCache, JInPos, EActivation::Activate);
    }
    else
    {
        GetPhysicsSystem()->GetBodyInterface().SetPosition(BodyIDCache, JInPos, EActivation::Activate);
    }
}

void MBodyObject::SetRotation(const ::Vec4& InRotation)
{
	JPH::Quat JInQuat = MJoltPhysics::DXQuatToJPHQuat(InRotation);

    Body& body = GetBody();
    GetPhysicsSystem()->GetBodyInterface().SetRotation(BodyIDCache, JInQuat, EActivation::Activate);
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
    JPH::Vec3 OutRot = GetPhysicsSystem()->GetBodyInterface().GetRotation(BodyIDCache).GetEulerAngles();

    return { OutRot.GetX(), OutRot.GetY(), OutRot.GetZ()};
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