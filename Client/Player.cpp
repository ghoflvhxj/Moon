#include "MoonEngine.h"

#include "DirectInput.h"
#include "MainGame.h"
#include "Camera.h"
#include "Player.h"
#include "Material.h"
#include "MeshComponent.h"
#include "StaticMeshComponent.h"
#include "DynamicMeshComponent.h"
#include "Texture.h"
#include "PointLightComponent.h"
#include "DirectionalLightComponent.h"
#include "SkyComponent.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"
#include "FBXLoader.h"

#include "imgui.h"

#include "Core/Serialize/JsonSerializer.h"
#include "Core/Serialize/JsonDeSerializer.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"

#define UsePointLight 1
#define UseRandomPointLight 1

#define UseDynamicMesh 1
#define UseSkySphere 0

using namespace DirectX;
using namespace rapidjson;

Player::Player()
	: Actor()
{
	initialize();
}

Player::~Player()
{
}

void Player::initialize()
{
    LoadedMeshComponent = std::make_shared<StaticMeshComponent>();
    LoadedMeshComponent->setScale(Vec3(0.01f, 0.01f, 0.01f));
    addComponent(TEXT("Load"), LoadedMeshComponent);

#if UsePointLight == 1
    _pLightComponent = std::make_shared<MPointLightComponent>();
    _pLightComponent->setRange(10.f);
    _pLightComponent->setTranslation(0.f, 0.f, 0.f);
    addComponent(TEXT("PointLight"), _pLightComponent);
#endif

#if UseDynamicMesh == 1
	CharacterMeshComponent = std::make_shared<DynamicMeshComponent>();
	addComponent(ROOT_COMPONENT, CharacterMeshComponent);
    CharacterMeshComponent->SetPhysics(false);
	CharacterMeshComponent->setTranslation(0.f, 0.f, 5.f);

    //CharacterMeshComponent->SetMesh(TEXT("2B/2b.json"));
    CharacterMeshComponent->SetMesh(TEXT("2B/2b.fbx"));
	CharacterMeshComponent->GetDynamicMesh()->getMaterial(3)->SetAlphaMask(true);
	CharacterMeshComponent->GetDynamicMesh()->getMaterial(4)->SetAlphaMask(true);
    // 2 = 치마

    CharacterMeshComponent->setDrawingBoundingBox(true);
    CharacterMeshComponent->SetAnimPlaying(false);
#endif

#if UseSkySphere == 1
	std::shared_ptr<MTexture> SkyTexture = std::make_shared<MTexture>(TEXT("./SkyDome/Hazy_Afternoon_Backplate_001.png"));
	_pSkyComponent = std::make_shared<SkyComponent>();
	_pSkyComponent->getSkyMesh()->getMaterial(0)->setTexture(ETextureType::Diffuse, SkyTexture);
	addComponent(TEXT("Sky"), _pSkyComponent);
	_pSkyComponent->setRotation(Vec3{ XMConvertToRadians(270.f), 0.f, 0.f });
#endif

#if UseRandomPointLight == 1
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> colorDis(0, 255);
	std::uniform_int_distribution<int> transDis(0, 10);

	for (int i = 0; i < 1; ++i)
	{
		std::shared_ptr<MPointLightComponent> pLight = std::make_shared<MPointLightComponent>();
		pLight->setTranslation(Vec3(transDis(gen) / 1.f, 1.f, transDis(gen) / 1.f));
		pLight->setColor(Vec3(colorDis(gen) / 255.f, colorDis(gen) / 255.f, colorDis(gen) / 255.f));
		pLight->setRange(10.f);
        //pLight->setIntensity(3.f);
		_pLightComponentList.push_back(pLight);

		std::wstring tag = std::wstring(TEXT("PointLightList")) + std::to_wstring(i);
		addComponent(tag.c_str(), pLight);
	}
#endif
}

void Player::initializeImGui()
{

}

void Player::tick(const Time deltaTime)
{
    if (auto CameraComponent = getMainGame()->getMainCamera()->getComponent(TEXT("RootComponent")))
    {
        Vec3 trans = CameraComponent->getTranslation();
        Vec3 look = CameraComponent->GetForward();
        Vec3 right = CameraComponent->getRight();
        float speed = CameraSpeedScale * 1.f * deltaTime;

        if (InputManager::keyPress(DIK_LSHIFT))
        {
            speed *= 5.f;
        }

        if (InputManager::keyPress(DIK_W))
        {
            trans.x += look.x * speed;
            trans.y += look.y * speed;
            trans.z += look.z * speed;
        }
        else if (InputManager::keyPress(DIK_S))
        {
            trans.x -= look.x * speed;
            trans.y -= look.y * speed;
            trans.z -= look.z * speed;
        }
        else if (InputManager::keyPress(DIK_D))
        {
            trans.x += right.x * speed;
            trans.y += right.y * speed;
            trans.z += right.z * speed;
        }
        else if (InputManager::keyPress(DIK_A))
        {
            trans.x -= right.x * speed;
            trans.y -= right.y * speed;
            trans.z -= right.z * speed;
        }

        CameraSpeedScale += static_cast<float>(InputManager::mouseMove(MOUSEAXIS::Z)) / 10.f;
        CameraSpeedScale = CameraSpeedScale >= 1.f ? CameraSpeedScale : 1.f;

        CameraComponent->setTranslation(trans);

        if (InputManager::mousePress(MOUSEBUTTON::RB))
        {
            Vec3 rot = CameraComponent->getRotation();

            float mouseX = static_cast<float>(InputManager::mouseMove(MOUSEAXIS::X));
            float mouseY = static_cast<float>(InputManager::mouseMove(MOUSEAXIS::Y));

            rot.x = rot.x + (((rot.x + mouseY) - rot.x) * 0.005f);
            rot.y = rot.y + (((rot.y + mouseX) - rot.y) * 0.005f);
            CameraComponent->setRotation(rot);
        }

    }

#if UseDynamicMesh == 1
	if (InputManager::keyPress(DIK_E))
	{
		CharacterMeshComponent->playAnimation(0, deltaTime);
	}
#endif

    static float DeltaTime = 0.f;
    DeltaTime += deltaTime / 2.f;

#if UsePointLight == 1
    _pLightComponent->setTranslation(std::cosf(0.f) * 5.f, 2.f, std::sinf(0.f) * 5.f);
#endif

#if UseRandomPointLight == 1
	if (InputManager::keyPress(DIK_P))
	{
		_pLightComponentList[0]->setTranslation(0.1f, 2.f, 4.f);
	}


	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> moveDis(-10, 10);
	if (!_pLightComponentList.empty())
	{
		for (int i = 0; i < _pLightComponentList.size(); ++i)
		{
			Vec3 trans = _pLightComponentList[i]->getTranslation();
			trans.x += moveDis(gen) * deltaTime;
			//trans.y += moveDis(gen) * deltaTime;
			trans.z += moveDis(gen) * deltaTime;

			_pLightComponentList[i]->setTranslation(trans);

            _pLightComponentList[i]->setTranslation(std::cosf(DeltaTime + 2.f) * 5.f, 2.f, std::sinf(DeltaTime + 2.f) * 5.f);
		}
	}
#endif
}

void Player::JsonSaveTest(bool bPretty)
{
    MJsonSerializer Serializer;

    std::wstring Path = TEXT("D:\\Git\\Moon\\Client\\test.json");

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
    std::wstring Path = TEXT("D:\\Git\\Moon\\Client\\test.json");

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
    Deserializer.Deserialize(*LoadedMeshComponent->GetMesh(), Path);
    LoadedMeshComponent->GetMesh()->LoadFromAsset(Path);

    //LoadedMeshComponent->SetMesh(Path);

    std::cout << "Deserialize Finished" << std::endl;
}