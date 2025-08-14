#include "TypeDesc.h"

std::set<const FTypeDesc*>& GetTypeDescs()
{
    static std::set<const FTypeDesc*> TypeDescs;
    return TypeDescs;
}