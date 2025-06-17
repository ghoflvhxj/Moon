#include "Include.h"
#include "MeshComponent.h"

#include "GraphicDevice.h"
#include "Material.h"

#include "Texture.h"

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

const bool MeshComponent::addTexture(std::shared_ptr<MTexture> pTexture)
{
	if (_textureList.size() == _textureList.capacity())
		return false;

	_textureList.push_back(pTexture);
	return true;
}

void MeshComponent::setMaterial(std::shared_ptr<MMaterial> pMaterial)
{
	_pMaterial = pMaterial;
}

std::shared_ptr<MMaterial>& MeshComponent::getMaterial()
{
	return _pMaterial;
}

void MeshComponent::setTexture(const ETextureType textureType, std::shared_ptr<MTexture> pTexture)
{
	_textureList[EnumToIndex(textureType)] = pTexture;

	// 매터리얼에 새로운 텍스쳐를 바인딩 해줌
	_pMaterial->setTextures(_textureList);
}

std::shared_ptr<MTexture> &MeshComponent::getTexture(const ETextureType textureType)
{
	return _textureList[EnumToIndex(textureType)];
}
