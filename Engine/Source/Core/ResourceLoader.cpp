#include "ResourceLoader.h"
#include "Texture.h"

MResourceLoader::~MResourceLoader()
{
	LoadedResources.clear();
}

const std::shared_ptr<MResource>& MResourceLoader::TryLoad(const std::wstring& FilePath)
{
	if (LoadedResources.find(FilePath) != LoadedResources.end())
	{
		return LoadedResources[FilePath];
	}

	if (const std::shared_ptr<MResource>& NewResource = MakeResource(FilePath))
	{
		LoadedResources[FilePath] = NewResource;
		return LoadedResources[FilePath];
	}

	static std::shared_ptr<MResource> Empty = nullptr;
	return Empty;
}

MTextureLoader::MTextureLoader()
	: MResourceLoader()
{
	Extensions.emplace(TEXT(".png"));
	Extensions.emplace(TEXT(".jpg"));
	Extensions.emplace(TEXT(".jpeg"));
	Extensions.emplace(TEXT(".tga"));
}

std::shared_ptr<MResource> MTextureLoader::MakeResource(const std::wstring& FilePath)
{
	return std::make_shared<MTexture>(FilePath);
}

