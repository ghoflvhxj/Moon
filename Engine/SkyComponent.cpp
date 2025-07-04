#include "SkyComponent.h"

#include "ShaderManager.h"

#include "Texture.h"
#include "Material.h"
#include "Renderer.h"
#include "Mesh/StaticMesh/StaticMesh.h"

using namespace DirectX;

SkyComponent::SkyComponent()
	: MPrimitiveComponent()
	, _baseColor{ 1.f, 1.f, 1.f }
{
	_pSkyMesh = std::make_shared<StaticMesh>();
	_pSkyMesh->LoadFromFBX(TEXT("SkyDome/SkyDome.fbx"));

	_pSkyMesh->getMaterial(0)->setShader(TEXT("TexVertexShader.cso"), TEXT("SkyPixelShader.cso"));
}

SkyComponent::~SkyComponent()
{
}

std::shared_ptr<StaticMesh> SkyComponent::getSkyMesh()
{
	return _pSkyMesh;
}

const bool SkyComponent::GetPrimitiveData(std::vector<FPrimitiveData> &primitiveDataList)
{
	if (nullptr == _pSkyMesh)
	{
		return false;
	}

    FPrimitiveData primitiveData = {};
	primitiveData.PrimitiveComponent = shared_from_this();
	primitiveData.PrimitiveType = EPrimitiveType::Sky;
    primitiveData.MeshData = _pSkyMesh->GetMeshData(0);
	primitiveData.Material = _pSkyMesh->getMaterials()[0];
	primitiveDataList.emplace_back(primitiveData);

	return true;
}

void SkyComponent::initialize()
{
	
}