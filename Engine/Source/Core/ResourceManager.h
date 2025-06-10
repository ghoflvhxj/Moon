#pragma once

#include "Include.h"
#include "ResourceLoader.h"

class MResourceLoader;

class MResourceManager
{
public:
	MResourceManager() = default;
	~MResourceManager() = default;

public:
	template <class T>
	bool Load(const std::wstring& InPath, std::shared_ptr<T>& OutResource)
	{
		std::filesystem::path Path(InPath);
		if (std::filesystem::exists(Path) == false)
		{
			return false;
		}
		
		if (Path.empty())
		{
			return false;
		}

		const std::wstring& FileExtension = Path.extension().wstring();
		if (ResourceLoaders.find(FileExtension) == ResourceLoaders.end())
		{
			MSGBOX(TEXT("지원되지 않은 파일 확장자(") + FileExtension + TEXT(")"));
			return false;
		}

		//if (std::is_same(T, ResourceLoaders[FileExtension]->ResourceType) == false)
		//{
		//	return nullptr;
		//}

		OutResource = std::static_pointer_cast<T>(ResourceLoaders[FileExtension]->TryLoad(Path));
		return OutResource != nullptr;
	}

	void AddLoader(const std::shared_ptr<MResourceLoader>& ResourceLoader);
	void Release();

protected:
	std::map<std::wstring, std::shared_ptr<MResourceLoader>> ResourceLoaders;
};