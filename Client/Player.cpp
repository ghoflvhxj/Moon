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

#include "imgui.h"

#include "Core/Serialize/JsonSerializer.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"

#define UsePointLight 1
#define UseRandomPointLight 1
#define UseDirectionalLight 1
#define UseDynamicMesh 1
#define UseSkySphere 0
#define UseGround 1

using namespace DirectX;
using namespace rapidjson;

Player::Player()
	: Actor()
{
	initialize();
}

Player::~Player()
{
	std::cout << "asd" << std::endl;
}

void Player::initialize()
{
#if UseGround == 1
	_pMeshComponent = std::make_shared<StaticMeshComponent>();
    _pMeshComponent->SetMesh(TEXT("Base/Box.fbx"));
	_pMeshComponent->getStaticMesh()->getMaterial(0)->setTexture(ETextureType::Diffuse, std::make_shared<MTexture>(TEXT("./Resources/Texture/stone_01_albedo.jpg")));
    _pMeshComponent->getStaticMesh()->getMaterial(0)->setTexture(ETextureType::Normal, std::make_shared<MTexture>(TEXT("./Resources/Texture/Stone_01_normal.jpg")));
	addComponent(ROOT_COMPONENT, _pMeshComponent);
	_pMeshComponent->setScale(20.f, 1.f, 20.f);
	_pMeshComponent->setTranslation(1.f, -3.f, 0.f);
#endif

    _pStaticMeshComponent2 = std::make_shared<StaticMeshComponent>();
    _pStaticMeshComponent2->SetMesh(TEXT("Table/Table.fbx"));
    _pStaticMeshComponent2->setScale(Vec3{ 0.02f, 0.02f, 0.02f });
    _pStaticMeshComponent2->setDrawingBoundingBox(true);
    _pStaticMeshComponent2->SetDrawCollision(true);
    addComponent(TEXT("test2"), _pStaticMeshComponent2);

    LoadedMeshComponent = std::make_shared<StaticMeshComponent>();
    addComponent(TEXT("Load"), LoadedMeshComponent);

#if UsePointLight == 1
    _pLightComponent = std::make_shared<PointLightComponent>();
    _pLightComponent->setRange(10.f);
    _pLightComponent->setTranslation(0.f, 0.f, 0.f);
    addComponent(TEXT("PointLight"), _pLightComponent);
#endif

#if UseDynamicMesh == 1
	CharacterMeshComponent = std::make_shared<DynamicMeshComponent>();
    CharacterMeshComponent->SetMesh(TEXT("2B/2b.fbx"));
	CharacterMeshComponent->setTranslation(0.f, 0.f, 5.f);
	addComponent(TEXT("DynamicMesh"), CharacterMeshComponent);
	CharacterMeshComponent->getDynamicMesh()->getMaterial(3)->SetAlphaMask(true);
	CharacterMeshComponent->getDynamicMesh()->getMaterial(4)->SetAlphaMask(true);
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

#if UseDirectionalLight == 1
	_pLightComponent2 = std::make_shared<DirectionalLightComponent>();
	addComponent(TEXT("DirectionalLight"), _pLightComponent2);
	_pLightComponent2->setTranslation(0.f, 0.f, 1000.f);
#endif

#if UseRandomPointLight == 1
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> colorDis(0, 255);
	std::uniform_int_distribution<int> transDis(0, 10);

	for (int i = 0; i < 1; ++i)
	{
		std::shared_ptr<PointLightComponent> pLight = std::make_shared<PointLightComponent>();
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

#if UseDirectionalLight == 1
	Vec3 rotation2 = _pLightComponent2->getRotation();
	if (InputManager::keyPress(DIK_UP))
	{
		rotation2.x += DirectX::XMConvertToRadians(10.f) * deltaTime;
		_pLightComponent2->setRotation(rotation2);
	}
#endif
}

void Player::JsonSaveTest(bool bPretty)
{

    Document Doc;
    Doc.SetObject();
    auto& Allocator = Doc.GetAllocator();

    /*
    std::shared_ptr<DynamicMesh> Mesh = CharacterMeshComponent->getDynamicMesh();
    uint32 MeshNum = Mesh->GetMeshNum();
    Value JsonMeshNum(kObjectType);
    JsonMeshNum.SetInt(MeshNum);
    Doc.AddMember("MeshNum", JsonMeshNum, Allocator);

    for (int i = 0; i < MeshNum; ++i)
    {
        auto Vec2Serialize = [&Allocator](const Vec2& InVec, Value& JsonValue) {
            JsonValue.PushBack(InVec.x, Allocator);
            JsonValue.PushBack(InVec.y, Allocator);
        };

        auto Vec3Serialize = [&Allocator](const Vec3& InVec, Value& JsonValue) {
            JsonValue.PushBack(InVec.x, Allocator);
            JsonValue.PushBack(InVec.y, Allocator);
            JsonValue.PushBack(InVec.z, Allocator);
        };

        auto Vec4Serialize = [&Allocator](const Vec4& InVec, Value& JsonValue) {
            JsonValue.PushBack(InVec.x, Allocator);
            JsonValue.PushBack(InVec.y, Allocator);
            JsonValue.PushBack(InVec.z, Allocator);
            JsonValue.PushBack(InVec.w, Allocator);
         };

        auto VectorSerialize = [&Allocator](auto& In, Value& JsonValue) {
            for (auto& Value : In)
            {
                JsonValue.PushBack(Value, Allocator);
            }
        };

        std::shared_ptr<FMeshData> MeshData = Mesh->GetMeshData(i);

        // MeshData Serialize
        Value JsonMesh(kObjectType);
        {
            Value JsonVertexNum;
            JsonVertexNum.SetInt(MeshData->Vertices.size());
            JsonMesh.AddMember("VertexNum", JsonVertexNum, Allocator);

            Value Indices(kArrayType);
            VectorSerialize(MeshData->Indices, Indices);
            JsonMesh.AddMember("Indices", Indices, Allocator);

            Value Pos(kArrayType);
            Value UV(kArrayType);
            Value Normal(kArrayType);
            Value Tangent(kArrayType);
            Value BiTangent(kArrayType);
            Value BlendIndices(kArrayType);
            Value BlendWeights(kArrayType);
            for (const Vertex& Vtx : MeshData->Vertices)
            {
                Vec4Serialize(Vtx.Pos, Pos);
                Vec2Serialize(Vtx.Tex0, UV);
                Vec3Serialize(Vtx.Normal, Normal);
                Vec3Serialize(Vtx.Tangent, Tangent);
                Vec3Serialize(Vtx.Binormal, BiTangent);
                VectorSerialize(Vtx.BlendIndex, BlendIndices);
                Vec4Serialize(Vtx.BlendWeight, BlendWeights);
            }

            JsonMesh.AddMember("Pos", Pos, Allocator);
            JsonMesh.AddMember("UV", UV, Allocator);
            JsonMesh.AddMember("Normal", Normal, Allocator);
            JsonMesh.AddMember("Tangent", Tangent, Allocator);
            JsonMesh.AddMember("BiTangent", BiTangent, Allocator);
            JsonMesh.AddMember("BlendIndices", BlendIndices, Allocator);
            JsonMesh.AddMember("BlendWeights", BlendWeights, Allocator);
        }

        const std::string str = "Mesh" + std::to_string(i);
        Value Name(str, Allocator);
        Doc.AddMember(Name, JsonMesh, Allocator);
    }
    */

    MJsonSerializer Serializer;

    // 정점 배열 저장 테스트
    //Vertex Test[2];
    //Test[0].Pos = { 1.f, 2.f, 3.f, 4.f };
    //Test[1].Pos = { 5.f, 6.f, 7.f, 8.f };
    //Value JsonMesh = Serializer.TSerialize(Test);
    //Value JsonMesh = Serializer.TSerialize(*_pStaticMeshComponent2->getStaticMesh()->GetMeshData(0));
    //Value JsonMesh = Serializer.TSerialize(*_pStaticMeshComponent2->getStaticMesh());
    Value JsonMesh = Serializer.TSerialize(*CharacterMeshComponent->getDynamicMesh());
    Doc.AddMember("Result", JsonMesh, Allocator);

	FILE* fp = nullptr;
	fopen_s(&fp, "D:\\Git\\Moon\\Client\\test.json", "wb");

	char WriteBuffer[65536];
	FileWriteStream WriteStream(fp, WriteBuffer, sizeof(WriteBuffer));

    if (bPretty)
    {
        PrettyWriter<FileWriteStream> JsonWriter(WriteStream);
        JsonWriter.SetMaxDecimalPlaces(8);
        Doc.Accept(JsonWriter);
    }
    else
    {
        Writer<FileWriteStream> JsonWriter(WriteStream);
        JsonWriter.SetMaxDecimalPlaces(8);
        Doc.Accept(JsonWriter);
    }

	fclose(fp);
}

void Player::JsonLoadTest()
{
    //LoadedMeshComponent->SetMesh(TEXT("D:\\Git\\Moon\\Client\\test.json"));
    //LoadedMeshComponent->setScale(Vec3(0.01f, 0.01f, 0.01f));

    CharacterMeshComponent->SetMesh(TEXT("D:\\Git\\Moon\\Client\\test.json"));
    /*
    auto ReadArray = [](Value& InValue, auto& OutVector) {
        for (auto& Value : InValue.GetArray())
        {
            OutVector.push_back(Value.GetInt());
        }
    };

    auto ReadArrayRange = [](Value& InValue, auto& Out, uint32 Start, uint32 Num) {
        for (uint32 i = 0; i < Num; ++i)
        {
            Out[Start + i] = InValue.GetArray()[Start + i].GetInt();
        }
    };

    auto ReadVec4 = [](Value& InValue, Vec4& Vec, uint32 Start) {
        Vec.x = InValue.GetArray()[Start * 4].GetFloat();
        Vec.y = InValue.GetArray()[Start * 4 + 1].GetFloat();
        Vec.z = InValue.GetArray()[Start * 4 + 2].GetFloat();
        Vec.w = InValue.GetArray()[Start * 4 + 3].GetFloat();
    };

    auto ReadVec3 = [](Value& InValue, Vec3& Vec, uint32 Start) {
        Vec.x = InValue.GetArray()[Start * 3].GetFloat();
        Vec.y = InValue.GetArray()[Start * 3 + 1].GetFloat();
        Vec.z = InValue.GetArray()[Start * 3 + 2].GetFloat();
    };

    auto ReadVec2 = [](Value& InValue, Vec2& Vec, uint32 Start) {
        Vec.x = InValue.GetArray()[Start * 2].GetFloat();
        Vec.y = InValue.GetArray()[Start * 2 + 1].GetFloat();
    };

    FILE* fp = nullptr;
    fopen_s(&fp, "D:\\Git\\Moon\\Client\\test.json", "rb");

    char readBuffer[2048];
    FileReadStream frs(fp, readBuffer, sizeof(readBuffer));

    Document Doc;
    Doc.ParseStream(frs);

    std::vector< std::shared_ptr<FMeshData>> MeshDatas;
    
    if (Doc.HasMember("MeshNum"))    
    {
        uint32 MeshNum = Doc["MeshNum"].GetInt();
        for (uint32 MeshCounter = 0; MeshCounter < MeshNum; ++MeshCounter)
        {
            std::string MeshName = "MeshNum" + std::to_string(MeshCounter);
            if (Doc.HasMember(MeshName))
            {
                std::shared_ptr<FMeshData> NewMeshData = std::make_shared<FMeshData>();

                ReadArray(Doc[MeshName]["Indices"], NewMeshData->Indices);

                uint32 VertexNum = Doc[MeshName]["VertexNum"].GetInt();
                NewMeshData->Vertices.resize(VertexNum);
                for (uint32 VertexCounter = 0; VertexCounter < VertexNum; ++VertexCounter)
                {
                    ReadVec4(Doc[MeshName]["Pos"], NewMeshData->Vertices[VertexCounter].Pos, VertexCounter);
                    ReadVec2(Doc[MeshName]["UV"], NewMeshData->Vertices[VertexCounter].Tex0, VertexCounter);
                    ReadVec3(Doc[MeshName]["Normal"], NewMeshData->Vertices[VertexCounter].Normal, VertexCounter);
                    ReadVec3(Doc[MeshName]["Tangent"], NewMeshData->Vertices[VertexCounter].Tangent, VertexCounter);
                    ReadVec3(Doc[MeshName]["BiTangent"], NewMeshData->Vertices[VertexCounter].Binormal, VertexCounter);
                    ReadArrayRange(Doc[MeshName]["BlendIndices"], NewMeshData->Vertices[VertexCounter].BlendIndex, VertexCounter * 4, 4);
                    ReadVec4(Doc[MeshName]["BlendWeights"], NewMeshData->Vertices[VertexCounter].BlendWeight, VertexCounter);
                }

                MeshDatas.push_back(NewMeshData);
            }
        }
    }

    fclose(fp);
    */
}

//void Player::rideTerrain(std::shared_ptr<TerrainComponent> pTerrainComponent)
//{
//	Vec3 trans	= _pMeshComponent->getTranslation();
//	Vec3 look	= _pMeshComponent->getLook();
//	Vec3 right	= _pMeshComponent->getRight();
//
//	float speed = 0.1f;
//
//	if (keyPress(DIK_UP))
//	{
//		trans.x += look.x * speed;
//		trans.z += look.z * speed;
//	}
//	else if (keyPress(DIK_DOWN))
//	{
//		trans.x -= look.x * speed;
//		trans.z -= look.z * speed;
//	}
//	else if (keyPress(DIK_RIGHT))
//	{
//		trans.x += right.x * speed;
//		trans.z += right.z * speed;
//	}
//	else if (keyPress(DIK_LEFT))
//	{
//		trans.x -= right.x * speed;
//		trans.z -= right.z * speed;
//	}
//
//	float y = 0.f;
//	if (pTerrainComponent->Test(trans, &trans.y))
//	{
//		_pMeshComponent->setTranslation({ trans.x, trans.y, trans.z });
//	}
//
//}