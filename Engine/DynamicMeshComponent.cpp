#include "DynamicMeshComponent.h"
#include "DynamicMeshComponentUtility.h"

#include "MoonEngine.h"
#include "Renderer.h"

#include "Core/Physics/Physics.h"
#include "Module/Physics/CharacterPhysics.h"

#include "Core/ResourceManager.h"
#include "Render.h"
#include "GraphicDevice.h"
#include "Material.h"
#include "Texture.h"

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

    if (IsAnimPlaying())
    {
        uint32 JointNum = Mesh->GetJointNum();
        if (Animation)
        {
            for (int32 JointIndex = 0; JointIndex < CastValue<int32>(JointNum); ++JointIndex)
            {
                float FloatFrame = AnimTime * 24.f;
                uint32 Frame = CastValue<uint32>(FloatFrame);

                XMMATRIX JointMatrix = XMLoadFloat4x4(&Animation->GetKeyFrame(Frame).GetJointMatrix(JointIndex));

                XMMATRIX NonScale = XMMatrixScaling(1.f / 2.54f, 1.f / 2.54f, 1.f / 2.54f);
                //JointMatrix = XMMatrixMultiply(NonScale, JointMatrix);

                // 다음 프레임과 블렌딩
                /*
                if (Frame < Animation->TotalFrame - 1)
                {
                    float currentFrameFactor = 1.f - (FloatFrame - CastValue<float>(Frame));
                    float nextFrameFactor = 1.f - currentFrameFactor;
                    XMMATRIX CurrentMat = XMLoadFloat4x4(&Animation->GetKeyFrame(Frame).GetJointMatrix(JointIndex));
                    XMMATRIX NextMat = XMLoadFloat4x4(&Animation->GetKeyFrame(Frame + 1).GetJointMatrix(JointIndex));
                    JointMatrix = (NextMat * nextFrameFactor) + (CurrentMat * currentFrameFactor);
                }
                */

                // 현재 프레임에서 조인트 행렬들
                XMMATRIX BindPoseInverseMatrix = XMLoadFloat4x4(&Mesh->GetJoint(JointIndex)._globalBindPoseInverseMatrix);
                //XMStoreFloat4x4(&JointAnimMatrices[JointIndex], XMMatrixMultiply(BindPoseInverseMatrix, JointMatrix));
                XMStoreFloat4x4(&JointAnimMatrices[JointIndex], BindPoseInverseMatrix  * JointMatrix * NonScale);
            }
        }

        playAnimation(AinmClipIndex, deltaTime);
    }
    
    static float t = 0.f;
    t += deltaTime;

    uint32 ClothPhysicsObjectNum = GetSize(ClothPhysicsObjects);
    uint32 ClothUpdateDataNum = GetSize(ClothUpdateDatas);
    if (ClothPhysicsObjectNum == ClothUpdateDataNum && ClothPhysicsObjectNum > 0 && ClothUpdateDataNum > 0)
    {
        for (uint32 ClothIndex = 0; ClothIndex < ClothPhysicsObjectNum; ++ClothIndex)
        {
            std::shared_ptr<MPhysicsObject>& ClothObject = ClothPhysicsObjects[ClothIndex];
            FClothUpdateData& ClothUpdateData = ClothUpdateDatas[ClothIndex];
            const FClothData& ClothData = GetDynamicMesh()->GetClothDatas()[ClothUpdateData.ClothDataIndex];

            //static Vec3 JointPos = GetJointPosition(JointName);
            //ClothObject->SetPos(JointPos);

            //Vec4 Quat = GetJointQuaternion(JointName);
            //ClothObject->SetRotation(Quat);

            // 조인트 위치 변화를 옷에 포스 적용
            //Vec3 Dir = VEC3ZERO;
            //XMStoreFloat3(&Dir, XMVector3Normalize(XMLoadFloat3(&ClothUpdateData.PreviousJointPos) - XMLoadFloat3(&JointPos)));
            //ClothUpdateData.PreviousJointPos = JointPos;
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
        NewPrimitiveData.AnimMatrices = JointAnimMatrices;

        if (geometryIndex == 7 || geometryIndex == 8 || geometryIndex == 11 || geometryIndex == 12 || geometryIndex == 13)
        {
            //static std::vector<Mat4> Test(200, IDENTITYMATRIX);
            //NewPrimitiveData.AnimMatrices = Test.data();
        }

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
    std::filesystem::path Path = MFIleSystem::AbsolutePath(InPath);

    if (Path.extension() == TEXT(".fbx"))
    {
        Mesh->LoadFromFBX(Path);
    }
    else if (Path.extension() == TEXT(".json"))
    {
        g_ResourceManager->Load(InPath, Mesh);
    }

    Materials = Mesh->getMaterials();

    OnMeshChangedDelegate.Broadcast(GetShared());
    OnPrimitiveChangedDelegate.Broadcast(GetShared());
}

void DynamicMeshComponent::Clothing()
{
    if (g_pPhysics == nullptr)
    {
        return;
    }

    MMeshComponent::Clothing();

    // 몸체
    if (GetDynamicMesh()->GetPhysics())
    {
        if (std::shared_ptr< MDynamicMeshPhysics> DynamicMeshPhysics = GetDynamicMesh()->GetPhysics()->CastTo<MDynamicMeshPhysics>())
        {
            for (FBodyCapsuleData& BodyCapsuleData : DynamicMeshPhysics->GetCapsules())
            {
                g_pPhysics->AddCharacterBody(GetShared(), BodyCapsuleData);
            }
        }
    }


    Clothing2();

    //// 옷 바디 충돌 테스트
    //{
    //    static std::shared_ptr<MPhysicsObject> BodyTestObject = nullptr;
    //    FBodyConstructData Data;
    //    Data.PrimitiveComponent = GetShared();
    //    Data.PhysicsType = EPhysicsType::Kinematic;
    //    g_pPhysics->AddPhysicsObject(Data, BodyTestObject);
    //}
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
            a.MeshIndex = 7;
            a.InvMass.resize(Mesh->GetMeshData(7).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            CothDatas.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 8;
            a.InvMass.resize(Mesh->GetMeshData(8).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            CothDatas.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 11;
            a.InvMass.resize(Mesh->GetMeshData(11).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            CothDatas.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 12;
            a.InvMass.resize(Mesh->GetMeshData(12).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            CothDatas.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 13;
            a.InvMass.resize(Mesh->GetMeshData(13).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            CothDatas.push_back(a);
        }

        GetDynamicMesh()->GetClothDatas() = CothDatas;

        g_pPhysics->AddCloth(Data, GetDynamicMesh()->GetClothDatas(), NewClothPhysicsObject);
        ClothPhysicsObjects.push_back(NewClothPhysicsObject);

        FClothUpdateData UpdateData = {};
        UpdateData.ClothDataIndex = 0;
        ClothUpdateDatas.push_back(UpdateData);
    }
}

Vec3 DynamicMeshComponent::GetJointPosition(const std::string& InName)
{
    return GetJointPosition(GetDynamicMesh()->GetJointIndex(InName));
}

Mat4 DynamicMeshComponent::GetJointMatrix(uint32 InJointIndex)
{
    XMVECTOR Trans = XMLoadFloat3(&VEC3ZERO);
    XMMATRIX WorldMat = XMLoadFloat4x4(&getWorldMatrix());

    Mat4 OutMat = IDENTITYMATRIX;
    if (Animation)
    {
        float FloatFrame = AnimTime * 24.f;
        uint32 Frame = CastValue<uint32>(FloatFrame);

        const FJoint& Joint = GetDynamicMesh()->GetJoint(InJointIndex);
        XMMATRIX XMNonScale = XMMatrixScalingFromVector(XMLoadFloat3(&VEC3ONE) / XMLoadFloat3(&Joint.Scale));

        XMMATRIX JointMatrix = XMLoadFloat4x4(&Animation->GetKeyFrame(Frame).GetJointMatrix(InJointIndex));
        XMMATRIX Matrix = JointMatrix * XMNonScale * WorldMat;
        Matrix = JointMatrix * WorldMat;

        XMStoreFloat4x4(&OutMat, Matrix);
    }

    return OutMat;
}

Vec3 DynamicMeshComponent::GetJointAxis(uint32 InJointIndex, uint32 InAxisIndex)
{
    const Mat4& Matrix = GetJointMatrix(InJointIndex);
    auto Temp = Matrix.m[InJointIndex];
    return { Temp[0], Temp[1], Temp[2] };
}

Vec3 DynamicMeshComponent::GetJointPosition(uint32 InJointIndex)
{
    Vec3 OutTrans = VEC3ZERO;

    if (InJointIndex < 0 || Mesh->GetJointNum() - 1 < InJointIndex)
    {
        return OutTrans;
    }

    XMVECTOR Trans = XMLoadFloat3(&VEC3ZERO);

    XMMATRIX Matrix = XMLoadFloat4x4(&GetJointMatrix(InJointIndex));
    XMVECTOR Dummy = {};
    XMMatrixDecompose(&Dummy, &Dummy, &Trans, Matrix);

    XMStoreFloat3(&OutTrans, Trans);

    return OutTrans;
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

    XMVECTOR Quat = {};
    XMMATRIX WorldMat = XMLoadFloat4x4(&getWorldMatrix());

    if (Animation)
    {
        float FloatFrame = AnimTime * 24.f;
        uint32 Frame = CastValue<uint32>(FloatFrame);

        const FJoint& Joint = GetDynamicMesh()->GetJoint(InJointIndex);
        XMMATRIX XMNonScale = XMMatrixScalingFromVector(XMLoadFloat3(&VEC3ONE) / XMLoadFloat3(&Joint.Scale));

        XMMATRIX JointMatrix = XMLoadFloat4x4(&Animation->GetKeyFrame(Frame).GetJointMatrix(InJointIndex));
        XMMATRIX Matrix = JointMatrix * WorldMat;

        XMVECTOR Dummy = {};
        XMMatrixDecompose(&Dummy, &Quat, &Dummy, Matrix);

        XMStoreFloat4(&OutQuat, Quat);

        /*
        XMMATRIX Matrix = XMLoadFloat4x4(&GetJointMatrix(InJointIndex));
        
        XMVECTOR Dummy = {};
        XMMatrixDecompose(&Dummy, &Quat, &Dummy, Matrix);

        XMStoreFloat4(&OutQuat, Quat);
        */
    }
    else
    {
        FJoint Joint = GetDynamicMesh()->GetJoint(InJointIndex);
        XMMATRIX JointMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&Joint.Scale)) * XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Joint.Rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&Joint.Position));
        XMMATRIX Matrix = JointMatrix * WorldMat;

        XMVECTOR Dummy = {};
        XMMatrixDecompose(&Dummy, &Quat, &Dummy, Matrix);

        XMStoreFloat4(&OutQuat, Quat);
    }

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
    XMMATRIX WorldMat = XMLoadFloat4x4(&getWorldMatrix());

    if (Animation)
    {
        XMMATRIX JointMatrix = XMLoadFloat4x4(&JointAnimMatrices[InJointIndex]);
        XMMATRIX NonScale = XMMatrixScaling(1.f / 2.54f, 1.f / 2.54f, 1.f / 2.54f);
        XMMATRIX Matrix = JointMatrix * NonScale * WorldMat;

        XMVECTOR Dummy = {};
        XMMatrixDecompose(&Scale, &Dummy, &Dummy, Matrix);

        XMStoreFloat3(&OutScale, Scale);
    }
    else
    {
        FJoint Joint = GetDynamicMesh()->GetJoint(InJointIndex);
        XMMATRIX JointMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&Joint.Scale)) * XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Joint.Rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&Joint.Position));
        XMMATRIX Matrix = JointMatrix * WorldMat;

        XMVECTOR Dummy = {};
        XMMatrixDecompose(&Scale, &Dummy, &Dummy, Matrix);

        XMStoreFloat3(&OutScale, Scale);
    }

    return OutScale;
}

void DynamicMeshComponent::SetAnimClip(const uint32 Index)
{
    if (AinmClipIndex != Index)
    {
        AinmClipIndex = Index;
        AnimTime = 0.f;
    }
}

uint32 DynamicMeshComponent::GetAnimClipNum()
{
    if (Mesh)
    {
        return GetSize(GetDynamicMesh()->GetAnimClips());
    }

    return 0;
}

std::shared_ptr<DynamicMesh> DynamicMeshComponent::GetDynamicMesh()
{
	return std::static_pointer_cast<DynamicMesh>(Mesh);
}

void DynamicMeshComponent::playAnimation(const uint32 index, const Time deltaTime)
{
	AinmClipIndex = index;
	AnimTime += deltaTime * AnimSpeed;

    if(Animation)
    {
        if (AnimTime > static_cast<float>(Animation->Duration))
        {
            AnimTime = 0.f;
        }
    }
}

Mat4 DynamicMeshComponent::GetAnimMatrix(uint32 JointIndex)
{
    Mat4 OutMatrix = IDENTITYMATRIX;

    MAnimation CurrentAnimClip;
    if (GetDynamicMesh()->getAnimationClip(AinmClipIndex, CurrentAnimClip))
    {
        float RealFrame = AnimTime * 24.f;
        uint32 Frame = CastValue<uint32>(RealFrame);

        XMMATRIX XMOutMatrix = XMLoadFloat4x4(&CurrentAnimClip.GetKeyFrame(Frame).GetJointMatrix(JointIndex));

        // 다음 프레임과 블렌딩
        if (Frame < CurrentAnimClip.TotalFrame - 1)
        {
            float currentFrameFactor = 1.f - (RealFrame - CastValue<float>(Frame));
            float nextFrameFactor = 1.f - currentFrameFactor;
            XMMATRIX CurrentMat = XMLoadFloat4x4(&CurrentAnimClip.GetKeyFrame(Frame).GetJointMatrix(JointIndex));
            XMMATRIX NextMat = XMLoadFloat4x4(&CurrentAnimClip.GetKeyFrame(Frame + 1).GetJointMatrix(JointIndex));
            XMOutMatrix = (NextMat * nextFrameFactor) + (CurrentMat * currentFrameFactor);
        }

        XMStoreFloat4x4(&OutMatrix, XMOutMatrix);
    }

    return OutMatrix;

    //return JointAnimMatrices[JointIndex];
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