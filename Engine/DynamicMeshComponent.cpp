#include "DynamicMeshComponent.h"
#include "DynamicMeshComponentUtility.h"

#include "MoonEngine.h"
#include "Renderer.h"

#include "Module/Physics/Physics.h"
#include "Module/Physics/CharacterPhysics.h"

#include "Core/ResourceManager.h"
#include "Render.h"
#include "GraphicDevice.h"
#include "Material.h"
#include "Texture.h"

#undef min

using namespace DirectX;
const std::string JointName = "bone001";

float AngleBetweenVectorsDegrees(XMVECTOR a, XMVECTOR b)
{
    a = XMVector3Normalize(a);
    b = XMVector3Normalize(b);
    XMVECTOR d = XMVector3Dot(a, b);
    float dot = XMVectorGetX(d);
    dot = std::clamp(dot, -1.0f, 1.0f);
    return XMConvertToDegrees(acosf(dot));
}

DynamicMeshComponent::DynamicMeshComponent()
	: MMeshComponent()
{
    Mesh = std::make_shared<DynamicMesh>();

    for (auto& AnimMatrix : JointAnimMatrices)
    {
        AnimMatrix = IDENTITYMATRIX;
    }
}

DynamicMeshComponent::DynamicMeshComponent(const std::wstring& FilePath)
	: MMeshComponent()
{
	Mesh = std::make_shared<DynamicMesh>();
    SetMesh(FilePath);

    for (auto& AnimMatrix : JointAnimMatrices)
    {
        AnimMatrix = IDENTITYMATRIX;
    }
}

DynamicMeshComponent::~DynamicMeshComponent()
{
}

void DynamicMeshComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bPlayAnimAtBegin)
    {
        bAnimPlaying = true;
    }
}

void DynamicMeshComponent::Update(const Time deltaTime)
{
    MMeshComponent::Update(deltaTime);

    float AnimBlendTime = 0.f;
    bool bAnimChage = AnimBlendTime > 0.f;

    if (HasAnim() && IsAnimPlaying())
    {
        uint32 JointNum = Mesh->GetJointNum();

        for (int32 JointIndex = 0; JointIndex < CastValue<int32>(JointNum); ++JointIndex)
        {
            XMMATRIX XMJointMatrix = XMLoadFloat4x4(&GetBlendedJointMatrix(BlendData, JointIndex));
            // 애니메이션이 바뀌었다면, 두 애니메이션을 블렌딩 해줌
            if (bAnimChage)
            {

            }

            if (bRootMotion)
            {
                XMMATRIX XMRootMat = XMLoadFloat4x4(&GetBlendedJointMatrix(BlendData, 0));

                Mat4 Mat;
                XMStoreFloat4x4(&Mat, XMRootMat);
                Vec3 S, R, T;
                DecomposeTransform(Mat, S, R, T);

                T.x = 0.f;
                T.y = 0.f;
                XMRootMat = XMMatrixTranslationFromVector(XMLoadFloat3(&T));

                XMJointMatrix = XMJointMatrix * XMMatrixInverse(nullptr, XMRootMat);
            }

            // 현재 프레임에서 조인트 행렬들
            const XMMATRIX& XMInvBindPoseMatrix = XMLoadFloat4x4(&Mesh->GetJoint(JointIndex)._globalBindPoseInverseMatrix);
            XMStoreFloat4x4(&JointAnimMatrices[JointIndex], XMInvBindPoseMatrix * XMJointMatrix);

            //getRenderer()->DrawCoordinate(GetWorld(), GetJointPosition(JointIndex), GetJointQuaternion(JointIndex));
        }

        /***********************
        [0, 10]
        F: 9.7      F: 9    P: 9    N: 10   PF: 0.3     NF: 0.7
        F: 10.7     F: 10   P: 10   N: 0    PF: 0.3     NF: 0.7
        ***********************/

        FloatFrame = clamp(AnimTime * Animation->GetFrameRate(), 0.f, static_cast<float>(Animation->EndFrame));
        Frame = CastValue<uint32>(FloatFrame);
        BlendData.PrevFrame = std::min(Frame, Animation->EndFrame);
        BlendData.NextFrame = Frame < Animation->EndFrame ? Frame + 1 : 0;
        BlendData.PrevFrameFactor = 1.f - (FloatFrame - CastValue<float>(Frame));
        BlendData.NextFrameFactor = 1.f - BlendData.PrevFrameFactor;

        if (Animation)
        {
            AnimTime += deltaTime * AnimSpeed;
            bLooped = false;
            while (AnimTime > Animation->Duration)
            {
                AnimTime -= Animation->Duration;
                bLooped = true;
                //BlendData.Reset();
            }
        }

        //getRenderer()->DrawCoordinate(GetWorld(), GetJointPosition("bone018"), VEC3ONE);
    }

    uint32 ClothPhysicsObjectNum = GetSize(ClothPhysicsObjects);
    uint32 ClothUpdateDataNum = GetSize(ClothUpdateDatas);
    if (ClothPhysicsObjectNum == ClothUpdateDataNum && ClothPhysicsObjectNum > 0 && ClothUpdateDataNum > 0)
    {
        for (uint32 ClothIndex = 0; ClothIndex < ClothPhysicsObjectNum; ++ClothIndex)
        {
            std::shared_ptr<MPhysicsObject>& ClothObject = ClothPhysicsObjects[ClothIndex];
            FClothUpdateData& ClothUpdateData = ClothUpdateDatas[ClothIndex];
            const FClothData& ClothData = GetDynamicMesh()->GetClothDatas()[ClothUpdateData.ClothDataIndex];

            // 조인트 위치 변화를 옷에 포스 적용
            Vec3 JointPos = GetJointPosition(JointName);

            Vec3 Dir = VEC3ZERO;
            XMStoreFloat3(&Dir, XMVector3Normalize(XMLoadFloat3(&ClothUpdateData.PreviousJointPos) - XMLoadFloat3(&JointPos)));
            ClothUpdateData.PreviousJointPos = JointPos;
            //ClothObject->AddForce(Dir);
        }
    }
}

const bool DynamicMeshComponent::GetPrimitiveData(std::vector<FPrimitiveData> & PrimitiveDataList)
{
	if (nullptr == Mesh)
	{
		return false;
	}

    std::shared_ptr<DynamicMesh>dMesh = GetDynamicMesh();

	uint32 geometryCount = dMesh->GetMeshNum();
	
    PrimitiveDataList.reserve(geometryCount);
	for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
	{
		FPrimitiveData NewPrimitiveData = {};
		NewPrimitiveData.PrimitiveComponent = GetShared();
		NewPrimitiveData.PrimitiveType = EPrimitiveType::Mesh;
		NewPrimitiveData.MeshData = &dMesh->GetMeshData(geometryIndex);
		NewPrimitiveData.Material = dMesh->getGeometryLinkMaterialIndex().size() > 0 ? dMesh->getMaterials()[dMesh->getGeometryLinkMaterialIndex()[geometryIndex]] : dMesh->getMaterials()[0];

        PrimitiveDataList.emplace_back(NewPrimitiveData);
	}

    //std::shared_ptr<MBoundingBox>& BoundingBox = dMesh->GetBoundingBox();
    //if (BoundingBox && _bDrawBoundingBox)
    //{
    //    FPrimitiveData PrimitiveData = {};
    //    PrimitiveData.PrimitiveComponent = GetShared();
    //    PrimitiveData.PrimitiveType = EPrimitiveType::Collision;
    //    PrimitiveData.MeshData = BoundingBox->GetMeshData().get();
    //    PrimitiveData.Material = BoundingBox->getMaterial();

    //    PrimitiveDataList.emplace_back(PrimitiveData);
    //}

	return true;
}

void DynamicMeshComponent::SetMesh(const std::wstring& InPath)
{
    std::filesystem::path Path = MFileSystem::AbsolutePath(InPath);

    if (Path.extension() == TEXT(".fbx"))
    {
        Mesh->LoadFromFBX(Path);
    }
    else if (Path.extension() == TEXT(".json"))
    {
        g_ResourceManager->Load(InPath, Mesh);
    }

    JointAnimMatrices.clear();
    Materials.clear();

    if (Mesh)
    {
        JointAnimMatrices.resize(Mesh->GetJointNum(), IDENTITYMATRIX);
        Materials = Mesh->getMaterials();
    }

    OnMeshChangedDelegate.Broadcast(this);
    OnPrimitiveChangedDelegate.Broadcast(this);
}

std::shared_ptr<MMesh> DynamicMeshComponent::GetMesh()
{
    return Mesh;
}

std::shared_ptr<DynamicMesh> DynamicMeshComponent::GetDynamicMesh()
{
    return Mesh;
}

void DynamicMeshComponent::Clothing()
{
    if (g_pPhysics == nullptr)
    {
        return;
    }

    MMeshComponent::Clothing();

    // 몸체
    if (std::shared_ptr<MPhysics>& PhyiscsAsset = GetDynamicMesh()->GetPhysics())
    {
        if (std::shared_ptr< MDynamicMeshPhysics>& DynamicMeshPhysics = PhyiscsAsset->CastToShared<MDynamicMeshPhysics>())
        {
            for (FBodyCapsuleData& BodyCapsuleData : DynamicMeshPhysics->GetCapsules())
            {
                g_pPhysics->AddCharacterPhyscics(GetShared(), BodyCapsuleData);
            }
        }
    }

    Clothing2();
}

void DynamicMeshComponent::Clothing2()
{
    Vec3 JointPos = GetJointPosition(JointName);
    uint32 JointIndex = GetDynamicMesh()->GetJointIndex(JointName);

    // 옷. 임시 하드코딩
    {
        std::shared_ptr<MPhysicsObject> NewClothPhysicsObject = nullptr;

        FBodyConstructData Data;
        Data.Mesh = Mesh;
        Data.PrimitiveComponent = GetShared();
        Data.PhysicsType = EPhysicsType::Dynamic;
        Data.Pos = JointPos;
        XMStoreFloat4(&Data.Rot, XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&getRotation())));

        std::vector<FClothData> CothDatas;
        {
            FClothData a;
            a.MeshIndex = 8;
            a.InvMass.resize(Mesh->GetMeshData(7).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            CothDatas.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 9;
            a.InvMass.resize(Mesh->GetMeshData(8).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            CothDatas.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 12;
            a.InvMass.resize(Mesh->GetMeshData(11).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            CothDatas.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 13;
            a.InvMass.resize(Mesh->GetMeshData(12).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            CothDatas.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 14;
            a.InvMass.resize(Mesh->GetMeshData(13).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            CothDatas.push_back(a);
        }

        GetDynamicMesh()->GetClothDatas() = CothDatas;

        // 임시
        getGraphicDevice()->BuildMeshPrivateBuffers(GetPrimitiveID(), GetMesh());
        getRenderer()->Test(GetWorld()->GetID(), GetPrimitiveID(), GetMesh());

        g_pPhysics->AddCloth(Data, GetDynamicMesh()->GetClothDatas(), NewClothPhysicsObject);
        ClothPhysicsObjects.push_back(NewClothPhysicsObject);

        FClothUpdateData UpdateData = {};
        UpdateData.ClothDataIndex = 0;
        ClothUpdateDatas.push_back(UpdateData);
    }
}

Mat4 DynamicMeshComponent::GetJointMatrix(uint32 InJointIndex, bool bOption)
{
    Mat4 OutMat = IDENTITYMATRIX;
    if (Animation)
    {
        const XMMATRIX& XMInvBindPoseMatrix = XMLoadFloat4x4(&Mesh->GetJoint(InJointIndex)._globalBindPoseInverseMatrix);
        XMMATRIX XMJointAnimMat = XMLoadFloat4x4(&JointAnimMatrices[InJointIndex]);                                 // JointAnimMatrix  =   InvBindPoseMatrix * JointMatrix, Update함수에서 계산되었음
        XMMATRIX XMJointMat = XMMatrixInverse(nullptr, XMInvBindPoseMatrix) * XMJointAnimMat;                       // OutMatrix        =   JointMatrix

        if (bOption)
        {
            const Mat4& RootJointMat = GetBlendedJointMatrix(BlendData, 0);
            XMJointMat = XMJointAnimMat * XMLoadFloat4x4(&RootJointMat);
        }

        XMStoreFloat4x4(&OutMat, XMJointMat);
    }

    return OutMat;
}

Mat4 DynamicMeshComponent::GetJointWorldMatrix(const std::string& InName)
{
    return GetJointWorldMatrix(Mesh->GetJointIndex(InName));
}

Mat4 DynamicMeshComponent::GetJointWorldMatrix(uint32 InJointIndex)
{
    XMVECTOR Trans = XMLoadFloat3(&VEC3ZERO);
    XMMATRIX XMWorldMat = XMLoadFloat4x4(&getWorldMatrix());

    Mat4 OutMat = IDENTITYMATRIX;
    if (Animation)
    {
        XMMATRIX XMJointWorldMat = XMLoadFloat4x4(&GetJointMatrix(InJointIndex)) * XMWorldMat;
        XMStoreFloat4x4(&OutMat, XMJointWorldMat);
    }
    else
    {
        const FJoint& Joint = GetDynamicMesh()->GetJoint(InJointIndex);
        Vec3 Scale = Joint.Scale;
        Vec3 Rot = Joint.Rotation;
        Vec3 Trans = Joint.Position;

        Mat4 JointMat = {};
        TransformMatrix(JointMat, Scale, Rot, Trans);

        XMStoreFloat4x4(&OutMat, XMLoadFloat4x4(&JointMat) * XMWorldMat);
    }

    return OutMat;
}

Mat4 DynamicMeshComponent::GetBlendedJointMatrix(const FBlendData& InBlendData, uint32 InJointIndex)
{
    Mat4 CurrentMat = Animation->GetKeyFrame(InBlendData.PrevFrame).GetJointMatrix(InJointIndex);
    Mat4 NextMat = Animation->GetKeyFrame(InBlendData.NextFrame).GetJointMatrix(InJointIndex);
    // EndFrame에서 0으로 갈때, 행렬이 매우 다를 수 있음
    // 0프레임 행렬의 위치를 EndFrame과 같게 한다면?
    // EndFrame의 행렬을 0프레임과 같게 한다면?
    if (InBlendData.NextFrame == 0)
    {
        //CurrentMat.m[3][0] = NextMat.m[3][0];
        //CurrentMat.m[3][1] = NextMat.m[3][1];
        //CurrentMat.m[3][2] = NextMat.m[3][2];
        //CurrentMat.m[3][3] = NextMat.m[3][3];
    }

    XMMATRIX XMCurrentMat = XMLoadFloat4x4(&CurrentMat);
    XMMATRIX XMNextMat = XMLoadFloat4x4(&NextMat);


    XMMATRIX XMJointMatrix = (XMNextMat * BlendData.NextFrameFactor) + (XMCurrentMat * BlendData.PrevFrameFactor);
   
    Mat4 OutMat = IDENTITYMATRIX;
    XMStoreFloat4x4(&OutMat, XMJointMatrix);

    return OutMat;
}

const FJoint& DynamicMeshComponent::GetJoint(const std::string& InName)
{
    return GetJoint(Mesh->GetJointIndex(InName));
}

const FJoint& DynamicMeshComponent::GetJoint(uint32 InJointIndex)
{
    if (Mesh)
    {
        return Mesh->GetJoint(InJointIndex);
    }

    return FJoint::Empty;
}

Vec3 DynamicMeshComponent::GetJointAxis(uint32 InJointIndex, uint32 InAxisIndex)
{
    const Mat4& Matrix = GetJointWorldMatrix(InJointIndex);
    auto Temp = Matrix.m[InJointIndex];
    return { Temp[0], Temp[1], Temp[2] };
}

Vec3 DynamicMeshComponent::GetJointPosition(uint32 InJointIndex, const Vec3& InOffset)
{
    Vec3 OutTrans = VEC3ZERO;

    if (InJointIndex < 0 || Mesh->GetJointNum() - 1 < InJointIndex)
    {
        return OutTrans;
    }

    const Mat4& JointWorldMat = GetJointWorldMatrix(InJointIndex);

    Mat4 Mat = {};
    XMStoreFloat4x4(&Mat, XMMatrixTranslationFromVector(XMLoadFloat3(&InOffset)) * XMLoadFloat4x4(&JointWorldMat));

    Vec3 S, R, T;
    DecomposeTransform(Mat, S, R, T);

    return T;
}

Vec3 DynamicMeshComponent::GetJointPosition(const std::string& InName, const Vec3& InOffset)
{
    return GetJointPosition(GetDynamicMesh()->GetJointIndex(InName), InOffset);
}

Vec3 DynamicMeshComponent::GetRelativeJointPosition(const std::string& InName)
{
    Vec3 OutPos = VEC3ZERO;

    XMStoreFloat3(&OutPos, XMLoadFloat3(&GetJointPosition(InName)) - XMLoadFloat3(&getWorldTranslation()));

    return OutPos;
}

Vec4 DynamicMeshComponent::GetJointQuaternion(const std::string& InName)
{
    return GetJointQuaternion(GetDynamicMesh()->GetJointIndex(InName));
}

Vec4 DynamicMeshComponent::GetJointQuaternion(uint32 InJointIndex)
{
    Vec4 OutQuat = VEC4ZERO;

    if (InJointIndex < 0 || Mesh->GetJointNum() - 1 < InJointIndex)
    {
        return OutQuat;
    }
    
    const FJoint& Joint = GetJoint(InJointIndex);

    XMVECTOR XMQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&VEC3ZERO));
    XMQuat = XMQuaternionNormalize(XMQuat);

    XMMATRIX XMQuatMat = XMMatrixRotationQuaternion(XMQuat) * XMLoadFloat4x4(&JointAnimMatrices[InJointIndex]) * XMLoadFloat4x4(&getWorldMatrix());

    XMVECTOR Dummy, Target;
    XMMatrixDecompose(&Dummy, &Target, &Dummy, XMQuatMat);

    Target =XMQuaternionNormalize(Target);
    XMStoreFloat4(&OutQuat, Target);

    return OutQuat;
}

Vec3 DynamicMeshComponent::GetJointScale(uint32 InJointIndex)
{
    Vec3 OutScale = VEC3ONE;

    if (InJointIndex < 0 || Mesh->GetJointNum() - 1 < InJointIndex)
    {
        return OutScale;
    }

    XMVECTOR Scale = XMLoadFloat3(&VEC3ZERO);
    XMVECTOR Dummy = {};
    XMMATRIX WorldMat = XMLoadFloat4x4(&getWorldMatrix());
    XMMATRIX Matrix = {};

    if (Animation)
    {
        XMMATRIX JointMatrix = XMLoadFloat4x4(&JointAnimMatrices[InJointIndex]);
        Matrix = JointMatrix * WorldMat;
    }
    else
    {
        FJoint Joint = GetDynamicMesh()->GetJoint(InJointIndex);
        XMMATRIX JointMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&Joint.Scale)) * XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Joint.Rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&Joint.Position));
        Matrix = JointMatrix * WorldMat;
    }

    XMMatrixDecompose(&Scale, &Dummy, &Dummy, Matrix);
    XMStoreFloat3(&OutScale, Scale);

    return OutScale;
}

Mat4 DynamicMeshComponent::GetAnimMatrix(const std::string& InName)
{
    return GetAnimMatrix(Mesh->GetJointIndex(InName));
}

Mat4 DynamicMeshComponent::GetAnimMatrix(uint32 JointIndex)
{
    return JointAnimMatrices[JointIndex];
}

bool DynamicMeshComponent::SetAnim(std::shared_ptr<MAnimation> InAnim)
{
    if (Animation == InAnim)
    {
        return false;
    }
    
    BlendData.Reset();
    Animation = InAnim;
    AnimTime = 0.f;

    return true;
}

/*
//bool bHasKeyFrames = !currentAnimClip._keyFrameLists[jointIndex][geometryIndex].empty();
//if (bHasKeyFrames == false)
//{
//
//    // 다른 메시에서 업데이트 된 경우
//    bool bUpdatedFromOtherMesh = matricesSet.find(jointIndex) != matricesSet.end();
//    if (bUpdatedFromOtherMesh)
//    {
//        continue;
//    }

//    // 본이 영향을 주는 버텍스가 없는 경우에는, 부모 본을 그대로 사용
//    int32 parentIndex = dMesh->getJoints()[jointIndex]._parentIndex;
//    if (parentIndex == -1)
//    {
//        _matrices[jointIndex] = IDENTITYMATRIX;
//    }
//    else
//    {
//        _matrices[jointIndex] = _matrices[parentIndex];
//    }
//
//}
//else
*/