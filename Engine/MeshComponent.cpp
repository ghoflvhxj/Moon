#include "Include.h"
#include "MeshComponent.h"

#include "GraphicDevice.h"
#include "Material.h"

#include "TextureComponent.h"

#include "MainGame.h"
#include "Camera.h"

#include "MainGame.h"
#include "MainGameSetting.h"

MeshComponent::MeshComponent()
	: PrimitiveComponent()
	, _textureList(5, nullptr)
{
	//initializeMeshInformation();
}

MeshComponent::~MeshComponent()
{
}

// 삭제예정
//void MeshComponent::initializeMeshInformation() 
//{
//	_vertexList.reserve(4);
//	_vertexList.push_back({ Vec3{ -0.5f, 0.5f, 1.f }, static_cast<Vec4>(Colors::White), Vec2{0.f, 0.f}, Vec3{0.f, 0.f, -1.f} });
//	_vertexList.push_back({ Vec3{ 0.5f, 0.5f, 1.f }, static_cast<Vec4>(Colors::Red), Vec2(1.f, 0.f), Vec3{0.f, 0.f, -1.f} });
//	_vertexList.push_back({ Vec3{ 0.5f, -0.5f, 1.f }, static_cast<Vec4>(Colors::Green), Vec2(1.f, 1.f), Vec3{0.f, 0.f, -1.f} });
//	_vertexList.push_back({ Vec3{ -0.5f, -0.5f, 1.f }, static_cast<Vec4>(Colors::Yellow), Vec2(0.f, 1.f), Vec3{0.f, 0.f, -1.f} });
//
//	_indexList.reserve(6);
//	_indexList.push_back(0);
//	_indexList.push_back(1);
//	_indexList.push_back(2);
//
//	_indexList.push_back(0);
//	_indexList.push_back(2);
//	_indexList.push_back(3);
//
//	_pMaterial = std::make_shared<Material>(_vertexList, _indexList);
//	_pMaterial->setTexture(_textureList);
//	_pMaterial->setShader(TEXT("TexVertexShader.cso"), TEXT("TexPixelShader.cso")); // 툴에서 설정한 쉐이더를 읽어야 하는데, 지금은 없으니까 그냥 임시로 땜빵
//}

//void MeshComponent::render()
//{
//	_pMaterial->render(shared_from_this());
//}

const bool MeshComponent::addTexture(std::shared_ptr<TextureComponent> pTexture)
{
	if (_textureList.size() == _textureList.capacity())
		return false;

	_textureList.push_back(pTexture);
	return true;
}

void MeshComponent::setMaterial(std::shared_ptr<Material> pMaterial)
{
	_pMaterial = pMaterial;
}

std::shared_ptr<Material>& MeshComponent::getMaterial()
{
	return _pMaterial;
}

void MeshComponent::setTexture(const TextureType textureType, std::shared_ptr<TextureComponent> pTexture)
{
	_textureList[enumToIndex(textureType)] = pTexture;

	// 매터리얼에 새로운 텍스쳐를 바인딩 해줌
	_pMaterial->setTexture(_textureList);
}

std::shared_ptr<TextureComponent> &MeshComponent::getTexture(const TextureType textureType)
{
	return _textureList[enumToIndex(textureType)];
}
