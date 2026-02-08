#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include "Macro.h"

/*
클래스, 구조체 같은 커스텀 자료형을 설명하는 구조체
이름, 부모 클래스, 사이즈, 어떤 멤버들을 가지고 있는지 등등
외부 라이브러리의 타입을 설명하려면 GetTypeDesc를 특수화 해주면 됨.
*/

struct FPropertyDesc;

struct FTypeDesc
{
	const FTypeDesc* Parent = nullptr;
	std::string Name;
	size_t Size = 0;

    // 멤버 정의를 담음
	std::vector<FPropertyDesc*> Properties;

    template <class T>
    bool IsA() const
    {
        const FTypeDesc* Current = this;

        while (Current)
        {
            if (Current == T::GetTypeDescStatic())
            {
                return true;
            }
            Current = Current->Parent;
        }

        return false;
    }
};

struct FFactoryBase
{
    virtual void* Create() { return nullptr; }
};

template <class T>
struct FFactorProxy : public FFactoryBase
{
    virtual void* Create() override
    {
        T* NewT = new T();
        return NewT;
    }
};

// TypeDesc를 저장하는 컨테이너 반환 
ENGINE_DLL std::map<std::string, const FTypeDesc*>& GetTypeDescs();
ENGINE_DLL std::map<const FTypeDesc*, FFactoryBase*>& GetFactory();

// TypeDesc 컨테이너에 추가
template <class T>
void AddTypeDesc(const FTypeDesc* InTypeDesc)
{
    const auto& Iter = GetTypeDescs().find(InTypeDesc->Name); 

    if (Iter == GetTypeDescs().end())
    {
        std::wstring Msg = TEXT("Add Type ") + StringToWString(InTypeDesc->Name);
        LOG(Msg);

        GetTypeDescs().emplace(InTypeDesc->Name, InTypeDesc);

        if constexpr (std::is_abstract_v<T> == false)
        {
            FFactoryBase* NewFactory = new FFactorProxy<T>();
            GetFactory()[InTypeDesc] = NewFactory;
        }
    }
    //else
    //{
    //    std::wstring Msg = TEXT("Overlapped Type ") + StringToWString(InTypeDesc->Name);
    //    LOG(Msg);
    //}
}

// 외부 타입의 Desc 생성을 위한 템플릿. 특수화하여 작업해야 함
template <class T>
const FTypeDesc* GetTypeDesc()
{
    return T::GetTypeDescStatic();
}

