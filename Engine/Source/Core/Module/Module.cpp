#include "Module.h"

MModule::~MModule()
{
    if (bReleased == false)
    {
        std::wstring Str = ModuleName + TEXT("모듈의 Release 함수를 파괴전에 호출하지 않았음!!!");
        LOG(Str);
    }
}

bool MModule::Initialize()
{
    const FTypeDesc* TypeDesc = GetTypeDesc();
    ModuleName = StringToWString(TypeDesc->Name);

    return true;
}

void MModule::Release()
{
    bReleased = true;
}
