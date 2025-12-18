#pragma once

#include "Include.h"

class MAsset;
struct FTypeDesc;

class ENGINE_DLL MResourceLoader
{
public:
	MResourceLoader() = default;
	MResourceLoader(const MResourceLoader& Rhs) = default;
	virtual ~MResourceLoader();

public:
	std::shared_ptr<MAsset> TryLoad(const std::wstring& FilePath);
    const std::unordered_map<std::wstring, std::shared_ptr<MAsset>>& GetLoadedResources() const { return LoadedResources; }
protected:
    // 경로, 리소스 쌍의 맵. 로드된 리로스가 여기에 저장됨
    std::unordered_map<std::wstring, std::shared_ptr<MAsset>> LoadedResources;
	
protected:
	virtual std::shared_ptr<MAsset> LoadAsset(const std::wstring& InPath) = 0;

    // 지원하는 확장자
public:
	const std::set<std::wstring>& GetExtensions() const { return Extensions; }
protected:
	std::set<std::wstring> Extensions;

    // 지원하는 TypeDesc
public:
    const FTypeDesc* GetSupportType() const { return TypeDesc; }
protected:
    const FTypeDesc* TypeDesc = nullptr;

protected:
    std::wstring DisplayName;
};

class ENGINE_DLL MTextureLoader : public MResourceLoader
{
public:
	MTextureLoader();
	MTextureLoader(const MTextureLoader& Rhs) = default;
protected:
	virtual std::shared_ptr<MAsset> LoadAsset(const std::wstring& InPath) override;
};

class ENGINE_DLL MMeshLoader : public MResourceLoader
{
public:
    MMeshLoader();

protected:
    virtual std::shared_ptr<MAsset> LoadAsset(const std::wstring& InPath) override;
};

class ENGINE_DLL MDynamicMeshLoader : public MResourceLoader
{
public:
    MDynamicMeshLoader();

protected:
    virtual std::shared_ptr<MAsset> LoadAsset(const std::wstring& InPath) override;
};

class ENGINE_DLL MMaterialLoader : public MResourceLoader
{
public:
    MMaterialLoader();

protected:
    virtual std::shared_ptr<MAsset> LoadAsset(const std::wstring& InPath) override;
};