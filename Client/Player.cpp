#include "stdafx.h"
#include "Player.h"

#include "Material.h"

#include "MeshComponent.h"
#include "StaticMeshComponent.h"
#include "DynamicMeshComponent.h"
#include "TextureComponent.h"
#include "PointLightComponent.h"
#include "DirectionalLightComponent.h"
#include "SkyComponent.h"
#include "Material.h"

#include "imgui.h"

#define UseLight 0
#define UseDirectionalLight 1
#define UseDynamicMesh 1
#define UseSkySphere 1
using namespace DirectX;

Player::Player()
{
	initialize();
}

Player::~Player()
{
}

void Player::initialize()
{
	_pTextureComponent = std::make_shared<TextureComponent>(TEXT("./Resources/Texture/Player.jpeg"));

	_pMeshComponent = std::make_shared<StaticMeshComponent>("Base/Box.fbx", true, true);
	_pMeshComponent->getStaticMesh()->getMaterial(0)->setTexture(TextureType::Diffuse, _pTextureComponent);
	addComponent(ROOT_COMPONENT, _pMeshComponent);
	_pMeshComponent->setScale(50.f, 1.f, 50.f);
	_pMeshComponent->setTranslation(1.f, -3.f, 20.f);
	_pMeshComponent->SetGravity(true);

	//_pStaticMeshComponent = std::make_shared<StaticMeshComponent>("Lantern/Lantern.fbx");
	//_pStaticMeshComponent->setScale(Vec3{ 0.01f, 0.01f, 0.01f });
	//_pStaticMeshComponent->setTranslation(0.f, 2.f, 0.f);
	//_pStaticMeshComponent->setDrawingBoundingBox(true);
	//addComponent(TEXT("test"), _pStaticMeshComponent);

	_pStaticMeshComponent2 = std::make_shared<StaticMeshComponent>("Table/Table.fbx", true, true);
	_pStaticMeshComponent2->setScale(Vec3{ 0.01f, 0.01f, 0.01f });
	_pStaticMeshComponent2->setDrawingBoundingBox(true);
	addComponent(TEXT("test2"), _pStaticMeshComponent2);

	_pLightComponent = std::make_shared<PointLightComponent>();
	addComponent(TEXT("PointLight"), _pLightComponent);

#if UseDynamicMesh == 1
	_pDynamicMeshComponent = std::make_shared<DynamicMeshComponent>("2B/2b.fbx");
	_pDynamicMeshComponent->setTranslation(0.f, 0.f, 3.f);
	_pDynamicMeshComponent->setScale(0.5f, 0.5f, 0.5f);
	addComponent(TEXT("DynamicMesh"), _pDynamicMeshComponent);
	for (int i = 0; i < _pDynamicMeshComponent->getDynamicMesh()->getMaterialCount(); ++i)
	{
		_pDynamicMeshComponent->getDynamicMesh()->getMaterial(i)->setShader(TEXT("TexAnimVertexShader.cso"), TEXT("TexPixelShader.cso"));
	}
#endif

#if UseSkySphere == 1
	std::shared_ptr<TextureComponent> _pTextureComponent2 = std::make_shared<TextureComponent>(TEXT("./SkyDome/Hazy_Afternoon_Backplate_001.png"));
	_pSkyComponent = std::make_shared<SkyComponent>();
	_pSkyComponent->getSkyMesh()->getMaterial(0)->setTexture(TextureType::Diffuse, _pTextureComponent2);
	addComponent(TEXT("Sky"), _pSkyComponent);
	_pSkyComponent->setRotation(Vec3{ XMConvertToRadians(270.f), 0.f, 0.f });
#endif

#if UseDirectionalLight == 1
	_pLightComponent2 = std::make_shared<DirectionalLightComponent>();
	addComponent(TEXT("DirectionalLight"), _pLightComponent2);
	_pLightComponent2->setRotation(Vec3{ XMConvertToRadians(45.f), 0.f, 0.f });
#endif
	//_pCollisionShapeComponent = std::make_shared<CollisionShapeComponent>();
	//addComponent(TEXT("CollisionShape"), _pCollisionShapeComponent);

	//_pBoneShapeComponent = std::make_shared<CollisionShapeComponent>(_pDynamicMeshComponent->_originBoneVertexList);
	//addComponent(TEXT("BoneShape"), _pBoneShapeComponent);

#if UseLight == 1
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> colorDis(0, 255);
	std::uniform_int_distribution<int> transDis(0, 100);

	for (int i = 0; i < 50; ++i)
	{
		std::shared_ptr<PointLightComponent> pLight = std::make_shared<PointLightComponent>();
		pLight->setTranslation(Vec3(transDis(gen) / 1.f, 1.f, transDis(gen) / 1.f));
		pLight->setColor(Vec3(colorDis(gen) / 255.f, colorDis(gen) / 255.f, colorDis(gen) / 255.f));
		pLight->setRange(3.f);
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
	if (keyPress(DIK_E))
	{
		_pDynamicMeshComponent->playAnimation(0, deltaTime);
	}
#endif


#if UseLight == 1
	if (keyPress(DIK_P))
	{
		_pLightComponentList[0]->setTranslation(0.1f, 2.f, 4.f);
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> moveDis(-10, 10);
	if (!_pLightComponentList.empty())
	{
		for (int i = 0; i < 50; ++i)
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
	if (keyPress(DIK_UP))
	{
		rotation2.x += DirectX::XMConvertToRadians(10.f) * deltaTime;
		_pLightComponent2->setRotation(rotation2);
	}
#endif
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