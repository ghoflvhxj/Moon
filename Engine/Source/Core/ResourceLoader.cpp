#include "ResourceLoader.h"

#include "Core/Reflection/TypeDesc.h"

#include "Texture.h"
#include "Mesh/StaticMesh/StaticMesh.h"

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

	if (std::shared_ptr<MAsset> NewResource = MakeResource(FilePath))
	{
		LoadedResources[FilePath] = NewResource;
		return LoadedResources[FilePath];
	}

    return nullptr;
}

MTextureLoader::MTextureLoader()
	: MResourceLoader()
{
	Extensions.emplace(TEXT(".png"));
	Extensions.emplace(TEXT(".jpg"));
	Extensions.emplace(TEXT(".jpeg"));
	Extensions.emplace(TEXT(".tga"));
}

std::shared_ptr<MAsset> MTextureLoader::MakeResource(const std::wstring& InPath)
{
	return std::make_shared<MTexture>(InPath);
}

MMeshLoader::MMeshLoader()
    : MResourceLoader()
{
    TypeDesc = StaticMesh::GetTypeDescStatic();
}

std::shared_ptr<MAsset> MMeshLoader::MakeResource(const std::wstring& InPath)
{
    auto& Mesh = std::make_shared<StaticMesh>();
    Mesh->SetAssetPath(InPath);
    Mesh->LoadFromAsset(InPath);

    return Mesh;
}

MMaterialLoader::MMaterialLoader()
    : MResourceLoader()
{
    TypeDesc = MMaterial::GetTypeDescStatic();
}

std::shared_ptr<MAsset> MMaterialLoader::MakeResource(const std::wstring& InPath)
{
    auto& Mat = std::make_shared<MMaterial>();
    Mat->SetAssetPath(InPath);
    Mat->LoadFromAsset(InPath);

    return Mat;
}