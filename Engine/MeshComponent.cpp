#include "Include.h"
#include "MeshComponent.h"

#include "GraphicDevice.h"
#include "Material.h"

#include "Texture.h"

#include "MainGame.h"
#include "Camera.h"

#include "MainGame.h"
#include "MainGameSetting.h"

MMeshComponent::MMeshComponent()
	: MPrimitiveComponent()
	, _textureList(5, nullptr)
{
	//initializeMeshInformation();
}

MMeshComponent::~MMeshComponent()
{
}

const bool MMeshComponent::addTexture(std::shared_ptr<MTexture> pTexture)
{
	if (_textureList.size() == _textureList.capacity())
		return false;

	_textureList.push_back(pTexture);
	return true;
}

void MMeshComponent::setMaterial(std::shared_ptr<MMaterial> pMaterial)
{
	_pMaterial = pMaterial;
}

std::shared_ptr<MMaterial>& MMeshComponent::getMaterial()
{
	return _pMaterial;
}

void MMeshComponent::setTexture(const ETextureType textureType, std::shared_ptr<MTexture> pTexture)
{
	_textureList[EnumToIndex(textureType)] = pTexture;

	// 매터리얼에 새로운 텍스쳐를 바인딩 해줌
	_pMaterial->setTextures(_textureList);
}

std::shared_ptr<MTexture> &MMeshComponent::getTexture(const ETextureType textureType)
{
	return _textureList[EnumToIndex(textureType)];
}
