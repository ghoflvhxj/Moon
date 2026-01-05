#include "Object.h"
#include "Asset.h"
#include "Core/Reflection/TypeDesc.h"
#include "Core/Serialize/JsonDeserializer.h"
#include "Core/ResourceManager.h"
#include "Core/FileSystem.h"

void MObject::LoadFromDisk(const std::wstring& InPath)
{
    if (Load(InPath))
    {
        // OnLoaded(); Deserializer에서 호출해줌
    }
}

bool MObject::Load(const std::wstring& InPath)
{
    assert(!InPath.empty());
    assert(MFileSystem::IsExist(InPath));

    MJsonDeserializer Deserializer;
    Deserializer.Deserialize(shared_from_this(), InPath);

    return true;
}

void MObject::OnLoaded()
{
    // DoNothing
}

void MObject::Copy(MObject* InObject) const
{
    assert(InObject);

    const FTypeDesc* TypeDesc = GetTypeDesc();
    assert(TypeDesc);

    while (TypeDesc != nullptr)
    {
        for (FPropertyDesc* PropDesc : TypeDesc->Properties)
        {
            // TODO. vector 타입인 경우는 Num이 유효하지 않음
            for (uint32 i = 0; i < PropDesc->Num; ++i)  
            {
                if (PropDesc->IsA<MObject>())
                {
                    std::shared_ptr<MObject> Src = *static_cast<std::shared_ptr<MObject>*>(PropDesc->GetAsVoid(this, i));
                    std::shared_ptr<MObject> Dst = *static_cast<std::shared_ptr<MObject>*>(PropDesc->GetAsVoid(InObject, i));
                    
                    if (Src)
                    {
                        if (Dst == nullptr)
                        {
                            PropDesc->SetAsVoid(InObject, &Src, i);
                        }
                        else
                        {
                            Src->Copy(Dst.get());
                        }
                    }
                }
                else
                {
                    PropDesc->Copy(this, InObject);
                }
            }
        }

        TypeDesc = TypeDesc->Parent;
    }
}

std::shared_ptr<MObject> MObject::Duplicate()
{
    const FTypeDesc* TypeDesc = GetTypeDesc();
    std::shared_ptr<MObject> NewObject((MObject*)CreateObject(TypeDesc));

    while (TypeDesc != nullptr)
    {
        for (FPropertyDesc* PropDesc : TypeDesc->Properties)
        {
            if (PropDesc->ContainerType == EContainerType::None)
            {
                for (uint32 i = 0; i < PropDesc->Num; ++i)
                {
                    if (PropDesc->IsA<MAsset>())
                    {
                        PropDesc->SetAsVoid(NewObject.get(), PropDesc->GetAsVoid(this, i), i);
                    }
                    if (PropDesc->IsA<MObject>())
                    {
                        if (std::shared_ptr<MObject> PropObject = *static_cast<std::shared_ptr<MObject>*>(PropDesc->GetAsVoid(this, i)))
                        {
                            std::shared_ptr<MObject> DuplicatedProp = PropObject->Duplicate();
                            PropDesc->SetAsVoid(NewObject.get(), &DuplicatedProp, i);
                        }
                    }
                    else
                    {
                        PropDesc->Copy(this, NewObject.get(), i);
                    }
                }
            }
            else if (PropDesc->ContainerType == EContainerType::Vector)
            {
                if (FVectorPropertyDesc* VectorProp = static_cast<FVectorPropertyDesc*>(PropDesc))
                {
                    size_t Num = VectorProp->GetNum(this);
                    VectorProp->Resize(NewObject.get(), Num);
                    for (size_t i = 0; i < Num; ++i)
                    {
                        if (PropDesc->IsA<MAsset>())
                        {
                            VectorProp->Set(NewObject.get(), i, VectorProp->Get(this, i));
                        }
                        else if (PropDesc->IsA<MObject>())
                        {
                            if (std::shared_ptr<MObject> PropObject = *static_cast<std::shared_ptr<MObject>*>(VectorProp->Get(this, i)))
                            {
                                std::shared_ptr<MObject> DuplicatedProp = PropObject->Duplicate();
                                VectorProp->Set(NewObject.get(), i, &DuplicatedProp);
                            }
                        }
                        else
                        {
                            VectorProp->Set(NewObject.get(), i, VectorProp->Get(this, i));
                        }
                    }
                }
            }
        }

        TypeDesc = TypeDesc->Parent;
    }

    if (NewObject != nullptr)
    {
        NewObject->OnDuplicated(this);
    }

    return NewObject;
}

std::shared_ptr<MObject> MObject::GetOwner() const
{
    return Owner.lock();
}

void MObject::SetOwner(std::shared_ptr<MObject> InObject)
{
    Owner = InObject;
}