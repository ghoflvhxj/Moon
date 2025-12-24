#include "Player.h"

#include "MoonEngine.h"
#include "DirectInput.h"
#include "World.h"
#include "Camera.h"
#include "Material.h"
#include "Texture.h"
#include "MeshComponent.h"
#include "StaticMeshComponent.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"
#include "FBXLoader.h"
#include "Core/ResourceManager.h"
#include "Renderer.h"

#include "Core/Serialize/JsonSerializer.h"
#include "Core/Serialize/JsonDeSerializer.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"

using namespace DirectX;
using namespace rapidjson;

//#define UseSkySphere 1
//#if UseSkySphere == 1
//std::shared_ptr<MTexture> SkyTexture = nullptr;
//g_ResourceManager->Load(TEXT("SkyDome/Hazy_Afternoon_Backplate_001.png"), SkyTexture);
//_pSkyComponent = std::make_shared<SkyComponent>();
//_pSkyComponent->getSkyMesh()->getMaterial(0)->setTexture(ETextureType::Diffuse, SkyTexture);
//AddComponent(TEXT("Sky"), _pSkyComponent);
//_pSkyComponent->setRotation(Vec3{ XMConvertToRadians(270.f), 0.f, 0.f });
//#endif

Player::Player()
	: Super()
{
    CharacterMeshComponent = std::make_shared<DynamicMeshComponent>();
    CharacterMeshComponent->SetPhysics(false);
    CharacterMeshComponent->SetMesh(TEXT("2B/2B.json"));
    CharacterMeshComponent->setDrawingBoundingBox(true);
    AddComponent(TEXT("CharacterMesh"), CharacterMeshComponent);

    CameraComponent = std::make_shared<MCameraComponent>();
    AddComponent(TEXT("Camera"), CameraComponent);

    CapsuleComponent = std::make_shared<MCapsuleComponent>();
    CapsuleComponent->AddChildComponent(CharacterMeshComponent);
    CapsuleComponent->AddChildComponent(CameraComponent);
    AddComponent(ROOT_COMPONENT, CapsuleComponent);
}

Player::~Player()
{
}

void Player::BeginPlay()
{
    Super::BeginPlay();

    GetPostLoopDelegate().Add([&]() {
        CharacterMeshComponent->Clothing();
    });
}

void Player::tick(const Time deltaTime)
{
    Super::tick(deltaTime);

    Vec3 Input = {};

    if (auto World = GetWorld())
    {
        if (InputManager::keyPress(DIK_T, World->GetID()))
        {
            Input.z += 1.f;
        }
        if (InputManager::keyPress(DIK_G, World->GetID()))
        {
            Input.z -= 1.f;
        }
        if (InputManager::keyPress(DIK_F, World->GetID()))
        {
            Input.x -= 1.f;
        }
        if (InputManager::keyPress(DIK_H, World->GetID()))
        {
            Input.x += 1.f;
        }

        if (CharacterMeshComponent->IsAnimPlaying(WalkAnim) || CharacterMeshComponent->IsAnimPlaying(WalkToIdleAnim))
        {
            CharacterMeshComponent->bRootMotion = true;

            const Mat4& JointMat = CharacterMeshComponent->GetJointMatrix(0, true);

            Vec3 Pos, Dummy;
            DecomposeTransform(JointMat, Dummy, Dummy, Pos);

            if (bResetBonePose)
            {
                PrevBonePos = Pos;
                bResetBonePose = false;
            }

            XMVECTOR DeltaPos = XMLoadFloat3(&Pos) - XMLoadFloat3(&PrevBonePos);

            Vec3 A;
            XMStoreFloat3(&A, DeltaPos);
            A.z /= deltaTime;
            CapsuleComponent->AddMove(A);

            PrevBonePos = Pos;
            if (CharacterMeshComponent->IsLooped())
            {
                bResetBonePose = true;
            }
        }
        else
        {
            bResetBonePose = true;
            PrevBonePos = VEC3ZERO;
            CharacterMeshComponent->bRootMotion = false;
        }
    }

    // 애님
    if (XMVector3Equal(XMLoadFloat3(&Input), XMLoadFloat3(&VEC3ZERO)) == false)
    {
        if (CharacterMeshComponent->SetAnim(WalkAnim))
        {
            PrevBonePos = VEC3ZERO;
        }
    }
    else
    {
        if (CharacterMeshComponent->SetAnim(WalkToIdleAnim))
        {
            PrevBonePos = VEC3ZERO;
        }
    }

    // 카메라
    if (XMVector3Equal(XMLoadFloat3(&Input), XMLoadFloat3(&VEC3ZERO)) == false)
    {
        float BodyRotY = CapsuleComponent->getRotation().y;
        float CamRotY = CameraComponent->getRotation().y;
        float TargetRotY = CamRotY;
        if (Input.z < 0.f)
        {
            //cout << "TargetRotY: " << ToDegree(TargetRotY) << ", TargetRotY ADd: " << ToDegree(TargetRotY + ToRadian(180.f)) << endl;
            TargetRotY += ToRadian(180.f);
        }
        if (Input.x > 0.f)
        {
            TargetRotY += ToRadian(90.f);
        }
        if (Input.x < 0.f)
        {
            TargetRotY += ToRadian(-90.f);
        }
        TargetRotY = fmod(TargetRotY, ToRadian(360.f));

        XMVECTOR XMForward = XMLoadFloat3(&VEC3FORWARD);
        Vec3 A = {};
        Vec3 B = {};
        XMStoreFloat3(&A, XMVector3Rotate(XMForward, XMQuaternionRotationRollPitchYaw(0.f, BodyRotY, 0.f)));
        XMStoreFloat3(&B, XMVector3Rotate(XMForward, XMQuaternionRotationRollPitchYaw(0.f, TargetRotY, 0.f)));

        //const Vec3& A = CapsuleComponent->GetForward();
        //const Vec3& B = CameraComponent->GetForward();

        float CrossY = XMVectorGetY(XMVector3Cross(XMLoadFloat3(&B), XMLoadFloat3(&A)));
        float TurnDir = CrossY > 0.f ? -1.f : 1.f;
        float DeltaRot = std::atan2(B.z - A.z, B.x - A.x);
        if (B.x < A.x)
        {
            DeltaRot = std::atan2(A.z - B.z, A.x - B.x);
        }
        if (DeltaRot < 0.f)
        {
            //DeltaRot += PI2;
        }

        // 카메라와 캡슐이 방향이 다르면, 캡슐을 회전
        
        if (fabs(DeltaRot) > ToRadian(0.1f))
        {
            float AddRot = TurnDir * ToRadian(360.f) * deltaTime;
            float NewBodyRot = BodyRotY + AddRot;
            CapsuleComponent->AddRotation({ 0.f, AddRot, 0.f });

            //cout << "BodyRot: " << ToDegree(BodyRotY) << ", TargetRot: " << ToDegree(TargetRotY) << ", DeltaRot: " << ToDegree(DeltaRot) << endl;

            if (BodyRotY < TargetRotY)
            {
                if (NewBodyRot > TargetRotY)
                {
                    CapsuleComponent->SetRotation({ 0.f, TargetRotY, 0.f });
                }
            }
            if (BodyRotY > TargetRotY)
            {
                if (NewBodyRot < TargetRotY)
                {
                    CapsuleComponent->SetRotation({ 0.f, TargetRotY, 0.f });
                }
            }
        }
    }
    else
    {
        CharacterMeshComponent->SetAnim(WalkToIdleAnim);
    }
}

void Player::JsonSaveTest(bool bPretty)
{
    MJsonSerializer Serializer;

    std::wstring Path = TEXT("D:\\Git\\Moon\\Client\\test.fbx");

    // ----------------------------------------  int 배열 저장 테스트
    //struct FTest
    //{
    //    int Arr[2];

    //    REFLECT_TOP(
    //        FTest,
    //        PROPERTY(Arr)
    //    )
    //};
    //FTest t;
    //t.Arr[0] = 1;
    //t.Arr[1] = 10;
    //Serializer.Serialize(t, Path, bPretty);

    // ----------------------------------------  Vec3 배열 저장 테스트
    //struct FTest
    //{
    //    Vec3 Vertices[2];

    //    REFLECT_TOP(
    //        FTest,
    //        PROPERTY(Vertices)
    //    );
    //};
    //FTest t;
    //t.Vertices[0] = { 1.f, 2.f, 3.f };
    //t.Vertices[1] = { 5.f, 6.f, 7.f };
    //Serializer.Serialize(t, Path, bPretty);

    // ----------------------------------------  정점 배열 저장 테스트
    //struct FTest
    //{
    //    Vertex Vertices[2];

    //    REFLECT_TOP(
    //        FTest,
    //        PROPERTY(Vertices)
    //    );
    //};
    //FTest t;
    //t.Vertices[0].Pos = { 1.f, 2.f, 3.f, 4.f };
    //t.Vertices[1].Pos = { 5.f, 6.f, 7.f, 8.f };
    //Serializer.Serialize(t, Path, bPretty);
    
    // ---------------------------------------- 단일 메시 저장 테스트
    //Serializer.Serialize(*_pStaticMeshComponent2->GetMesh()->GetMeshData(0), Path, bPretty);
     
    // ---------------------------------------- 메시 저장 테스트
    //Serializer.Serialize(_pStaticMeshComponent2->GetMesh(), Path, bPretty);

    // ---------------------------------------- 
    //Serializer.Serialize(CharacterMeshComponent->GetDynamicMesh(), Path, bPretty);

    // ---------------------------------------- FBX를 로드해서 Json으로 저장
    MFBXLoader FbxLoader;
    FbxLoader.SaveJsonAsset(TEXT("2B/2B.fbx"));
}

void Player::JsonLoadTest()
{
    MJsonDeserializer Deserializer;
    std::wstring Path = TEXT("D:\\Git\\Moon\\Client\\Table\\table.fbx");

    // ----------------------------------------  int 배열 저장 테스트
    //struct FTest
    //{
    //    int Arr[2];

    //    REFLECT_TOP(
    //        FTest,
    //        PROPERTY(Arr)
    //    )
    //};
    //FTest t;
    //Deserializer.Deserialize(t, Path);

    // ----------------------------------------  Vec3 배열 저장 테스트
    //struct FTest
    //{
    //    Vec3 Vertices[2];

    //    REFLECT_TOP(
    //        FTest,
    //        PROPERTY(Vertices)
    //    );
    //};
    //FTest t;
    //Deserializer.Deserialize(t, Path);
 
    // ----------------------------------------  정점 배열 저장 테스트
    //struct FTest
    //{
    //    Vertex Vertices[2];

    //    REFLECT_TOP(
    //        FTest,
    //        PROPERTY(Vertices)
    //    );
    //};
    //FTest t;
    //Deserializer.Deserialize(t, Path);

    // ---------------------------------------- 단일 메시 저장 테스트
    //FMeshData MeshData;
    //Deserializer.Deserialize(MeshData, Path);

    // 메시 저장 테스트
    //Deserializer.Deserialize(*LoadedMeshComponent->GetMesh(), Path);
    
    //LoadedMeshComponent->GetMesh()->LoadFromAsset(Path);
    //LoadedStaticMeshComp->SetMesh(Path);

    std::cout << "Deserialize Finished" << std::endl;
}