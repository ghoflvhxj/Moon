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

std::shared_ptr<StaticMesh> SkyComponent::getSkyMesh()
{
	return _pSkyMesh;
}

const bool SkyComponent::getPrimitiveData(PrimitiveData &primitiveData)
{
	if (nullptr == _pSkyMesh)
	{
		return false;
	}

	primitiveData._pVertexBuffers = _pSkyMesh->getVertexBuffers();
	primitiveData._pIndexBuffer = _pSkyMesh->getIndexBuffer();
	primitiveData._pMaterials = _pSkyMesh->getMaterials();
	primitiveData._pVertexShader = _pSkyMesh->getMaterial(0)->getVertexShader();
	primitiveData._pPixelShader = _pSkyMesh->getMaterial(0)->getPixelShader();
	primitiveData._primitiveType = EPrimitiveType::Sky;

	return true;
}

void SkyComponent::initialize()
{
	
}