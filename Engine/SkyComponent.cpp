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
	_pSkyMesh->initializeMeshInformation("SkyDome/SkyDome.fbx", false);

	_pSkyMesh->getMaterial(0)->setShader(TEXT("TexVertexShader.cso"), TEXT("SkyPixelShader.cso"));
}

SkyComponent::~SkyComponent()
{
}

std::shared_ptr<StaticMesh> SkyComponent::getSkyMesh()
{
	return _pSkyMesh;
}

const bool SkyComponent::getPrimitiveData(std::vector<PrimitiveData> &primitiveDataList)
{
	if (nullptr == _pSkyMesh)
	{
		return false;
	}

	PrimitiveData primitiveData = {};
	primitiveData._pPrimitive = shared_from_this();
	primitiveData._pVertexBuffer = _pSkyMesh->getVertexBuffers()[0];
	primitiveData._pIndexBuffer = _pSkyMesh->getIndexBuffer();
	primitiveData._pMaterial = _pSkyMesh->getMaterials()[0];
	primitiveData._primitiveType = EPrimitiveType::Sky;
	primitiveDataList.emplace_back(primitiveData);

	return true;
}

void SkyComponent::initialize()
{
	
}