#include "DynamicMeshComponent.h"
#include "DynamicMeshComponentUtility.h"

#include "Core/ResourceManager.h"
#include "Core/Physics/Physics.h"
#include "Render.h"
#include "GraphicDevice.h"
#include "Material.h"
#include "Texture.h"

using namespace DirectX;

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

                // 다음 프레임과 블렌딩
                if (Frame < Animation->TotalFrame - 1)
                {
                    float currentFrameFactor = 1.f - (FloatFrame - CastValue<float>(Frame));
                    float nextFrameFactor = 1.f - currentFrameFactor;
                    XMMATRIX CurrentMat = XMLoadFloat4x4(&Animation->GetKeyFrame(Frame).GetJointMatrix(JointIndex));
                    XMMATRIX NextMat = XMLoadFloat4x4(&Animation->GetKeyFrame(Frame + 1).GetJointMatrix(JointIndex));
                    JointMatrix = (NextMat * nextFrameFactor) + (CurrentMat * currentFrameFactor);
                }

                // 현재 프레임에서 조인트 행렬들
                XMMATRIX BindPoseInverseMatrix = XMLoadFloat4x4(&Mesh->GetJoint(JointIndex)._globalBindPoseInverseMatrix);
                XMStoreFloat4x4(&JointAnimMatrices[JointIndex], XMMatrixMultiply(BindPoseInverseMatrix, JointMatrix));
            }
        }

        playAnimation(AinmClipIndex, deltaTime);
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

            // 옷 위치 업데이트
            Vec3 JointPos = GetJointPosition(ClothData.JointIndex);
            ClothObject->SetPos(JointPos);
            ClothObject->SetRotation(GetJointRotation(ClothData.JointIndex));

            // 조인트 위치 변화를 옷에 포스 적용
            Vec3 Dir = VEC3ZERO;
            XMStoreFloat3(&Dir, XMVector3Normalize(XMLoadFloat3(&ClothUpdateData.PreviousJointPos) - XMLoadFloat3(&JointPos)));
            ClothUpdateData.PreviousJointPos = JointPos;

            ClothObject->AddForce(Dir);
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

	if (dMesh->Skeleton)
	{
		//PrimitiveData primitive = {};
		//primitive._pPrimitive = shared_from_this();
		//primitive._pVertexBuffer = _pDynamicdMesh->_pSkeleton->getVertexBuffer();
		//primitive._pIndexBuffer = _pDynamicdMesh->_pSkeleton->getIndexBuffer();
		//primitive._pMaterial = _pDynamicdMesh->_pSkeleton->getMaterial();
		//primitive._primitiveType = EPrimitiveType::Mesh;

		//for (uint32 geometryIndex = 0; geometryIndex < geometryCount; ++geometryIndex)
		//{
		//	for (int32 jointIndex = 0; jointIndex < CastValue<int32>(jointCount); ++jointIndex)
		//	{

		//	}
		//}
		//primitive._matrices = _matrices;

		//primitiveDataList.emplace_back(primitive);
	}

    std::shared_ptr<MBoundingBox>& BoundingBox = dMesh->GetBoundingBox();
    if (BoundingBox && _bDrawBoundingBox)
    {
        FPrimitiveData PrimitiveData = {};
        PrimitiveData.PrimitiveComponent = GetShared();
        PrimitiveData.PrimitiveType = EPrimitiveType::Collision;
        PrimitiveData.MeshData = BoundingBox->GetMeshData().get();
        PrimitiveData.Material = BoundingBox->getMaterial();

        PrimitiveDataList.emplace_back(PrimitiveData);
    }

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
    MMeshComponent::Clothing();

    std::string JointName = "bone001";
    Vec3 JointPos = GetJointPosition(JointName);
    uint32 JointIndex = GetDynamicMesh()->GetJointIndex(JointName);
    // 옷
    if (g_pPhysics)
    {
        std::shared_ptr<MPhysicsObject> NewClothPhysicsObject = nullptr;

        FPhysicsConstructData Data;
        Data.Mesh = Mesh;
        Data.PrimitiveComponent = GetShared();
        Data.PhysicsType = EPhysicsType::Dynamic;
        Data.Pos = JointPos;

        std::vector<FClothData> t;
        {
            FClothData a;
            a.MeshIndex = 7;
            a.InvMass.resize(Mesh->GetMeshData(7).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            t.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 8;
            a.InvMass.resize(Mesh->GetMeshData(8).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            t.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 11;
            a.InvMass.resize(Mesh->GetMeshData(11).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            t.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 12;
            a.InvMass.resize(Mesh->GetMeshData(12).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            t.push_back(a);
        }
        {
            FClothData a;
            a.MeshIndex = 13;
            a.InvMass.resize(Mesh->GetMeshData(13).Vertices.size(), 1.f);
            a.JointIndex = JointIndex;
            t.push_back(a);
        }

        GetDynamicMesh()->GetClothDatas() = t;

        //g_pPhysics->AddCloth(Data, GetDynamicMesh()->GetClothDatas(), NewClothPhysicsObject);
        //ClothPhysicsObjects.push_back(NewClothPhysicsObject);

        FClothUpdateData UpdateData = {};
        UpdateData.ClothDataIndex = 0;
        ClothUpdateDatas.push_back(UpdateData);
    }

    // 옷 바디 충돌 테스트
    {
        //FPhysicsConstructData Data;
        //Data.PrimitiveComponent = shared_from_this();
        //Data.PhysicsType = EPhysicsType::Kinematic;
        //Data.bCapsule = true;
        //g_pPhysics->AddPhysicsObject(Data, BodyTestObject);
    }
}

void DynamicMeshComponent::SetPhysics(bool bInPhysics, bool bForce /* = false */)
{
    MMeshComponent::SetPhysics(bInPhysics, bForce);
}

Vec3 DynamicMeshComponent::GetJointPosition(const std::string& InName)
{
    return GetJointPosition(GetDynamicMesh()->GetJointIndex(InName));
}

Vec3 DynamicMeshComponent::GetJointPosition(uint32 JointIndex)
{
    Vec3 OutPos = VEC3ZERO;
    XMVECTOR Pos = XMLoadFloat3(&VEC3ZERO);
    XMMATRIX WorldMat = XMLoadFloat4x4(&getWorldMatrix());

    FJoint Joint = GetDynamicMesh()->GetJoint(JointIndex);
    MAnimation CurrentAnimClip;
    if (Animation && bAnimPlaying)
    {
        float RealFrame = AnimTime * 24.f;
        uint32 Frame = CastValue<uint32>(RealFrame);

        XMMATRIX JointMatrix = XMLoadFloat4x4(&Animation->GetKeyFrame(Frame).GetJointMatrix(JointIndex));
        Pos = XMVector3TransformCoord(Pos, JointMatrix);
        Pos = XMVector3TransformCoord(Pos, WorldMat);
        XMStoreFloat3(&OutPos, Pos);
    }
    else
    {
        XMMATRIX JointMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&Joint.Scale)) * XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&Joint.Rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&Joint.Position));
        Pos = XMVector3TransformCoord(Pos, JointMatrix);
        Pos = XMVector3TransformCoord(Pos, WorldMat);
        XMStoreFloat3(&OutPos, Pos);
    }


    return OutPos;
}

Vec3 DynamicMeshComponent::GetRelativeJointPosition(const std::string& InName)
{
    Vec3 OutPos = VEC3ZERO;

    XMStoreFloat3(&OutPos, XMLoadFloat3(&GetJointPosition(InName)) - XMLoadFloat3(&getWorldTranslation()));

    return OutPos;
}

Vec4 DynamicMeshComponent::GetJointRotation(const std::string& InName)
{
    return GetJointRotation(GetDynamicMesh()->GetJointIndex(InName));
}

Vec4 DynamicMeshComponent::GetJointRotation(uint32 JointIndex)
{
    Vec4 OutRot = VEC4ZERO;

    MAnimation CurrentAnimClip;
    if (GetDynamicMesh()->getAnimationClip(AinmClipIndex, CurrentAnimClip))
    {
        float RealFrame = AnimTime * 24.f;
        uint32 Frame = CastValue<uint32>(RealFrame);

        XMVECTOR Scale, Rotation, Translation;

        XMMatrixDecompose(&Scale, &Rotation, &Translation, XMLoadFloat4x4(&CurrentAnimClip.GetKeyFrame(Frame).GetJointMatrix(JointIndex)));
        XMStoreFloat4(&OutRot, Rotation);
    }

    return OutRot;
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
	AnimTime += deltaTime;

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