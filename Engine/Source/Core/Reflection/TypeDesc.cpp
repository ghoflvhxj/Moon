#include "TypeDesc.h"

std::map<std::string, const FTypeDesc*>& GetTypeDescs()
{
    static std::map<std::string, const FTypeDesc*> TypeDescs;
    return TypeDescs;
}

std::map<const FTypeDesc*, FFactoryBase*>& GetFactory()
{
    static std::map<const FTypeDesc*, FFactoryBase*> Factory;
    return Factory;
}

ENGINE_DLL void* Create(const FTypeDesc* InTypeDesc)
{
    if (GetFactory().find(InTypeDesc) != GetFactory().end())
    {
        return GetFactory()[InTypeDesc]->Create();
    }

    return nullptr;
}
