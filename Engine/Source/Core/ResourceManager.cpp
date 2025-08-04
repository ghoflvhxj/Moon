#include "ResourceManager.h"
#include "ResourceLoader.h"

MResourceManager::MResourceManager()
{
    AddLoader(std::make_shared<MTextureLoader>());
    AddLoader(std::make_shared<MMeshLoader>());
    AddLoader(std::make_shared<MMaterialLoader>());
}

void MResourceManager::AddLoader(const std::shared_ptr<MResourceLoader>& InLoader)
{
	for (const std::wstring& Extension : InLoader->GetExtensions())
	{
#if _DEBUG
		if (ResourceLoaders.find(Extension) != ResourceLoaders.end())
		{
			MSGBOX(TEXT("확장자의 리소스 로더가 덮어 씌워짐(") + Extension + TEXT(")"));
		}
#endif
		ResourceLoaders[Extension] = InLoader;
	}

    if (const FTypeDesc* TypeDesc = InLoader->GetSupportType())
    {
        ResourceLoaders2[TypeDesc] = InLoader;
    }
}

void MResourceManager::Release()
{
	ResourceLoaders.clear();
}