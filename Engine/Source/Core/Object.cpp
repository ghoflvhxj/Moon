#include "Object.h"
#include "Asset.h"
#include "Core/Reflection/TypeDesc.h"

#include "Core/Serialize/JsonDeserializer.h"
#include "Core/ResourceManager.h"

void MObject::LoadFromDisk(const std::wstring& InPath)
{
    if (Load(InPath))
    {
        // OnLoaded(); Deserializer에서 호출해줌
    }
}

bool MObject::Load(const std::wstring& InPath)
{
    if (InPath.empty())
    {
        return false;
    }

    std::filesystem::path FileSystemPath(InPath);
    if (FileSystemPath.extension() == TEXT(".json"))
    {
        MJsonDeserializer Deserializer;
        Deserializer.Deserialize(shared_from_this(), InPath);
    }

    return true;
}

void MObject::OnLoaded()
{
    // DoNothing
}

void MObject::Copy(MObject* InObject) const
{
    if (InObject == nullptr)
    {
        return;
    }

    const FTypeDesc* TypeDesc = GetTypeDesc();
    if (TypeDesc != InObject->GetTypeDesc())
    {
        return;
    }

    for (FPropertyDesc* PropDesc : TypeDesc->Properties)
    {
        PropDesc->Copy(this, InObject);
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
                    uint32 Num = VectorProp->GetNum(this);
                    VectorProp->Resize(NewObject.get(), Num);
                    for (uint32 i = 0; i < Num; ++i)
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