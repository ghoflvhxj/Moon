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