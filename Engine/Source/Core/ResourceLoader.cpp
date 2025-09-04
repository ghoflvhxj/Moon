#include "ResourceLoader.h"

#include "Core/Reflection/TypeDesc.h"

#include "Texture.h"
#include "Mesh/StaticMesh/StaticMesh.h"
#include "Mesh/DynamicMesh/DynamicMesh.h"

MResourceLoader::~MResourceLoader()
{
	LoadedResources.clear();
}

std::shared_ptr<MAsset> MResourceLoader::TryLoad(const std::wstring& FilePath)
{
	if (LoadedResources.find(FilePath) != LoadedResources.end())
	{
		return LoadedResources[FilePath];
	}

    std::wstring Msg = DisplayName + TEXT(" Asset Loading : ") + FilePath + TEXT("\r\n");
    LOG(Msg);

	if (std::shared_ptr<MAsset> NewResource = LoadAsset(FilePath))
	{
		LoadedResources[FilePath] = NewResource;
		return LoadedResources[FilePath];
	}

    return nullptr;
}

MTextureLoader::MTextureLoader()
	: MResourceLoader()
{
    DisplayName = TEXT("Texture");
	Extensions.emplace(TEXT(".png"));
	Extensions.emplace(TEXT(".jpg"));
	Extensions.emplace(TEXT(".jpeg"));
	Extensions.emplace(TEXT(".tga"));
}

std::shared_ptr<MAsset> MTextureLoader::LoadAsset(const std::wstring& InPath)
{
	return std::make_shared<MTexture>(InPath);
}

MMeshLoader::MMeshLoader()
    : MResourceLoader()
{
    DisplayName = TEXT("Mesh");
    TypeDesc = StaticMesh::GetTypeDescStatic();
}

std::shared_ptr<MAsset> MMeshLoader::LoadAsset(const std::wstring& InPath)
{
    auto& Mesh = std::make_shared<StaticMesh>();
    Mesh->LoadFromDisk(InPath);

    return Mesh;
}

MDynamicMeshLoader::MDynamicMeshLoader()
{
    DisplayName = TEXT("DynamicMesh");
    TypeDesc = DynamicMesh::GetTypeDescStatic();
}

std::shared_ptr<MAsset> MDynamicMeshLoader::LoadAsset(const std::wstring& InPath)
{
    auto& Mesh = std::make_shared<DynamicMesh>();
    Mesh->LoadFromDisk(InPath);

    return Mesh;
}


MMaterialLoader::MMaterialLoader()
    : MResourceLoader()
{
    DisplayName = TEXT("Material");
    TypeDesc = MMaterial::GetTypeDescStatic();
}

std::shared_ptr<MAsset> MMaterialLoader::LoadAsset(const std::wstring& InPath)
{
    auto& Mat = std::make_shared<MMaterial>();
    Mat->LoadFromDisk(InPath);

    return Mat;
}