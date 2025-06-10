#pragma once

#include "Include.h"

enum class EResourceType
{
	None,
	Texture,
	Mesh,
	Material,
	Animation,
	Shader,
	Count
};

class ENGINE_DLL MResource
{
public:
	MResource() = default;
	virtual ~MResource() = default;

protected:
	EResourceType ResourceType = EResourceType::None;
};

class ENGINE_DLL MResourceLoader
{
public:
	MResourceLoader() = default;
	MResourceLoader(const MResourceLoader& Rhs) = default;
	virtual ~MResourceLoader();

public:
	const std::shared_ptr<MResource>& TryLoad(const std::wstring& FilePath);
protected:
	std::unordered_map<std::wstring, std::shared_ptr<MResource>> LoadedResources;
	
protected:
	virtual std::shared_ptr<MResource> MakeResource(const std::wstring& FilePath) = 0;

public:
	const std::set<std::wstring>& GetExtensions() const { return Extensions; }
protected:
	std::set<std::wstring> Extensions;
};

class ENGINE_DLL MTextureLoader : public MResourceLoader
{
public:
	MTextureLoader();
	MTextureLoader(const MTextureLoader& Rhs) = default;
protected:
	virtual std::shared_ptr<MResource> MakeResource(const std::wstring& FilePath) override;
};