#include "stdafx.h"
#include "SkyComponent.h"

#include "ShaderManager.h"

#include "StaticMeshComponent.h"
#include "TextureComponent.h"
#include "Material.h"
#include "Renderer.h"

using namespace DirectX;

SkyComponent::SkyComponent()
	: PrimitiveComponent()
	, _baseColor{ 1.f, 1.f, 1.f }
{
	_pSkyMesh = std::make_shared<StaticMesh>();
	_pSkyMesh->initializeMeshInformation("SkyDome/SkyDome.fbx");

	_pSkyMesh->getMaterial(0)->setShader(TEXT("TexVertexShader.cso"), TEXT("SkyPixelShader.cso"));
}

SkyComponent::~SkyComponent()
{
}

const bool SkyComponent::getPrimitiveData(PrimitiveData &primitiveData)
{
	if (nullptr == _pSkyMesh)
	{
		return false;
	}

	primitiveData._pVertexBuffer = _pSkyMesh->getVertexBuffer();
	primitiveData._pIndexBuffer = _pSkyMesh->getIndexBuffer();
	primitiveData._pMaterial = _pSkyMesh->getMaterial(0);
	primitiveData._pVertexShader = _pSkyMesh->getMaterial(0)->getVertexShader();
	primitiveData._pPixelShader = _pSkyMesh->getMaterial(0)->getPixelShader();
	primitiveData._primitiveType = EPrimitiveType::Sky;

	return true;
}

void SkyComponent::initialize()
{
	
}