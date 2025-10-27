#pragma once

#include "Include.h"

class ENGINE_DLL MObject : public std::enable_shared_from_this<MObject>
{
public:
    MObject() = default;
    virtual ~MObject() = default;

public:
    // 디스크에 저장된 애셋을 불러올 때 사용하는 함수.
    virtual void LoadFromDisk(const std::wstring& InPath);
    // 경로만 설정되었을 떄 이 함수를 이용해 로드할 수 있음. 각 애셋들은 적절하게 오버라이딩 해야함
    virtual bool Load(const std::wstring& InPath);
    // 로드 성공 후 처리할 작업을 작성.
    virtual void OnLoaded();

public:
    template <class T>
    bool IsA()
    {
        return GetTypeDesc()->IsA<T>();
    }

    template <class T>
    std::shared_ptr<T> CastTo()
    {
        return IsA<T>() ? std::static_pointer_cast<T>(shared_from_this()) : nullptr;
    }

public:
    std::shared_ptr<MObject> GetOwner() const { return Owner.lock(); }
    void SetOwner(std::shared_ptr<MObject> InObject)
    {
        Owner = InObject;
    }


protected:
    std::weak_ptr<MObject> Owner;

    REFLECT_TOP(MObject);
};

template <class T>
std::shared_ptr<T> CastTo(std::shared_ptr<MObject> InObject)
{
    return InObject->IsA<T>() ? std::static_pointer_cast<T>(InObject) : nullptr;
}