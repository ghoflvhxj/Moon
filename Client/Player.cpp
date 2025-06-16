#include "Include.h"
#include "DirectInput.h"

#include "Player.h"

#include "Material.h"

#include "MeshComponent.h"
#include "StaticMeshComponent.h"
#include "DynamicMeshComponent.h"
#include "Texture.h"
#include "PointLightComponent.h"
#include "DirectionalLightComponent.h"
#include "SkyComponent.h"
#include "Material.h"

#include "imgui.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"

#define UsePointLight 1
#define UseRandomPointLight 1
#define UseDirectionalLight 1
#define UseDynamicMesh 1
#define UseSkySphere 1
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
	GroundTexture = std::make_shared<MTexture>(TEXT("./Resources/Texture/stone_01_albedo.jpg"));
	_pMeshComponent = std::make_shared<StaticMeshComponent>(TEXT("Base/Box.fbx"), true, true);
	_pMeshComponent->getStaticMesh()->getMaterial(0)->setTexture(ETextureType::Diffuse, GroundTexture);
	addComponent(ROOT_COMPONENT, _pMeshComponent);
	_pMeshComponent->setScale(20.f, 1.f, 20.f);
	_pMeshComponent->setTranslation(1.f, -3.f, 0.f);
	_pMeshComponent->SetGravity(true); // -> 머지?
#endif

	//_pStaticMeshComponent = std::make_shared<StaticMeshComponent>("Lantern/Lantern.fbx");
	//_pStaticMeshComponent->setScale(Vec3{ 0.01f, 0.01f, 0.01f });
	//_pStaticMeshComponent->setTranslation(0.f, 2.f, 0.f);
	//_pStaticMeshComponent->setDrawingBoundingBox(true);
	//addComponent(TEXT("test"), _pStaticMeshComponent);

	_pStaticMeshComponent2 = std::make_shared<StaticMeshComponent>(TEXT("Table/Table.fbx"), true, true);
	_pStaticMeshComponent2->setScale(Vec3{ 0.02f, 0.02f, 0.02f });
	_pStaticMeshComponent2->setDrawingBoundingBox(true);
	addComponent(TEXT("test2"), _pStaticMeshComponent2);

#if UsePointLight == 1
    _pLightComponent = std::make_shared<PointLightComponent>();
    _pLightComponent->setRange(10.f);
    _pLightComponent->setTranslation(0.f, 0.f, 0.f);
    //_pLightComponent->setIntensity(10.f);
    addComponent(TEXT("PointLight"), _pLightComponent);
#endif

#if UseDynamicMesh == 1
	_pDynamicMeshComponent = std::make_shared<DynamicMeshComponent>(TEXT("2B/2b.fbx"));
	_pDynamicMeshComponent->setTranslation(0.f, 0.f, 3.f);
	//_pDynamicMeshComponent->setScale(0.5f, 0.5f, 0.5f);
	addComponent(TEXT("DynamicMesh"), _pDynamicMeshComponent);
	//_pDynamicMeshComponent->getDynamicMesh()->getMaterial(0)->SetAlphaMask(true);
	//_pDynamicMeshComponent->getDynamicMesh()->getMaterial(1)->SetAlphaMask(true);
	//_pDynamicMeshComponent->getDynamicMesh()->getMaterial(2)->SetAlphaMask(true);
	_pDynamicMeshComponent->getDynamicMesh()->getMaterial(3)->SetAlphaMask(true);
	_pDynamicMeshComponent->getDynamicMesh()->getMaterial(4)->SetAlphaMask(true);
    _pDynamicMeshComponent->setDrawingBoundingBox(true);
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
	//_pCollisionShapeComponent = std::make_shared<CollisionShapeComponent>();
	//addComponent(TEXT("CollisionShape"), _pCollisionShapeComponent);

	//_pBoneShapeComponent = std::make_shared<CollisionShapeComponent>(_pDynamicMeshComponent->_originBoneVertexList);
	//addComponent(TEXT("BoneShape"), _pBoneShapeComponent);

#if UseRandomPointLight == 1
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> colorDis(0, 255);
	std::uniform_int_distribution<int> transDis(0, 10);

	for (int i = 0; i < 5; ++i)
	{
		std::shared_ptr<PointLightComponent> pLight = std::make_shared<PointLightComponent>();
		pLight->setTranslation(Vec3(transDis(gen) / 1.f, 1.f, transDis(gen) / 1.f));
		pLight->setColor(Vec3(colorDis(gen) / 255.f, colorDis(gen) / 255.f, colorDis(gen) / 255.f));
		pLight->setRange(1.f);
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
	//Vec3 trans = _pLightComponent->getTranslation();
	//Vec3 look = _pLightComponent->getLook();
	//Vec3 right = _pLightComponent->getRight();

	//float speed = 0.1f;

	//if (keyPress(DIK_UP))
	//{
	//	trans.x += look.x * speed;
	//	trans.z += look.z * speed;
	//}
	//else if (keyPress(DIK_DOWN))
	//{
	//	trans.x -= look.x * speed;
	//	trans.z -= look.z * speed;
	//}
	//else if (keyPress(DIK_RIGHT))
	//{
	//	trans.x += right.x * speed;
	//	trans.z += right.z * speed;
	//}
	//else if (keyPress(DIK_LEFT))
	//{
	//	trans.x -= right.x * speed;
	//	trans.z -= right.z * speed;
	//}



#if UseDynamicMesh == 1
	if (InputManager::keyPress(DIK_E))
	{
		_pDynamicMeshComponent->playAnimation(0, deltaTime);
	}
#endif


#if UsePointLight == 1
    static float DeltaTime = 0.f;
    DeltaTime += deltaTime;
    _pLightComponent->setTranslation(std::cosf(DeltaTime) * 5.f, std::sinf(DeltaTime), std::sinf(DeltaTime) * 5.f);
#endif

#if RandomPointLight == 1
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

void Player::Test(bool bPretty)
{
	Document Doc;
	Doc.SetObject();

	Value MaterialValue(kArrayType);
	const MaterialList& Materials = _pDynamicMeshComponent->getDynamicMesh()->getMaterials();
	for (const std::shared_ptr<MMaterial>& Mat : Materials)
	{
		//Mat->
	}

	uint32 MeshNum = _pDynamicMeshComponent->getDynamicMesh()->GetMeshNum();
	for (int i = 0; i < MeshNum; ++i)
	{
		Value Mesh(kObjectType);
		Value PosValue(kArrayType);
		Value TexValue(kArrayType);
		Value NormalValue(kArrayType);

		for (const Vertex& Vtx : _pDynamicMeshComponent->getDynamicMesh()->GetMeshData(i)->Vertices)
		{
			PosValue.PushBack(Vtx.Pos.x, Doc.GetAllocator());
			PosValue.PushBack(Vtx.Pos.y, Doc.GetAllocator());
			PosValue.PushBack(Vtx.Pos.z, Doc.GetAllocator());

			TexValue.PushBack(Vtx.Tex0.x, Doc.GetAllocator());
			TexValue.PushBack(Vtx.Tex0.y, Doc.GetAllocator());

			NormalValue.PushBack(Vtx.Normal.x, Doc.GetAllocator());
			NormalValue.PushBack(Vtx.Normal.y, Doc.GetAllocator());
			NormalValue.PushBack(Vtx.Normal.z, Doc.GetAllocator());
		}

		Mesh.AddMember("Pos", PosValue, Doc.GetAllocator());
		Mesh.AddMember("Tex", TexValue, Doc.GetAllocator());
		Mesh.AddMember("Normal", NormalValue, Doc.GetAllocator());

		uint32 MaterialNum = _pDynamicMeshComponent->getDynamicMesh()->GetMaterialNum();
		Value MaterialIndices(kArrayType);
		for (int j = 0; j < MaterialNum; ++j)
		{
			_pDynamicMeshComponent->getDynamicMesh()->getGeometryLinkMaterialIndex();
		}
		//Mesh.AddMember("Material", );

		const std::string str = "Mesh" + std::to_string(i);
		Value Name(str, Doc.GetAllocator());
		Doc.AddMember(Name, Mesh, Doc.GetAllocator());
	}

	Value TestArray(kArrayType);
	//Value TestArray;
	//TestArray.SetArray();
	for (int i = 0; i < 10; ++i)
	{
		TestArray.PushBack(0.1f * i, Doc.GetAllocator());
	}
	Doc.AddMember("TestArray", TestArray, Doc.GetAllocator());

	Value TestInt;
	TestInt.SetInt(10);
	Doc.AddMember("TestInt", TestInt, Doc.GetAllocator());

	FILE* fp = nullptr;
	fopen_s(&fp, "D:\\Git\\Moon\\Client\\test.json", "wb");

	char WriteBuffer[65536];
	FileWriteStream WriteStream(fp, WriteBuffer, sizeof(WriteBuffer));

	//Writer<FileWriteStream>* JsonWriter = nullptr;
	//if (bPretty)
	//{
	//	JsonWriter = new PrettyWriter<FileWriteStream>(WriteStream);
	//}
	//else
	//{
	//	JsonWriter = new Writer<FileWriteStream>(WriteStream);
	//}
	
	//JsonWriter->SetMaxDecimalPlaces(8);
	//Doc.Accept(*JsonWriter);
	//delete JsonWriter;

	PrettyWriter<FileWriteStream> JsonWriter(WriteStream);
	JsonWriter.SetMaxDecimalPlaces(8);
	Doc.Accept(JsonWriter);

	fclose(fp);
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