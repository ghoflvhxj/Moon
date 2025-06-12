#include "ResourceManager.h"

void MResourceManager::AddLoader(const std::shared_ptr<MResourceLoader>& ResourceLoader)
{
	for (const std::wstring& Extension : ResourceLoader->GetExtensions())
	{
#if _DEBUG
		if (ResourceLoaders.find(Extension) != ResourceLoaders.end())
		{
			MSGBOX(TEXT("확장자의 리소스 로더가 덮어 씌워짐(") + Extension + TEXT(")"));
		}
#endif

		ResourceLoaders[Extension] = ResourceLoader;
	}
}

void MResourceManager::Release()
{
	ResourceLoaders.clear();
}

