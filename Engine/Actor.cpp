#include "Actor.h"

#include "MoonEngine.h"
#include "MapUtility.h"

#include "Component.h"
#include "SceneComponent.h"

MActor::MActor()
	: SceneComponents()
{
}

MActor::~MActor()
{
    // DoNothing
}

void MActor::PostConstruct()
{
    Super::PostConstruct();

    const FTypeDesc* TypeDesc = GetTypeDesc();
    while (TypeDesc != nullptr)
    {
        for (FPropertyDesc* PropertyDesc : TypeDesc->Properties)
        {
            void* Data = PropertyDesc->GetAsVoid(this);
            assert(Data);

            if (PropertyDesc->IsA<MSceneComponent>()) // 씬 컴포넌트
            {
                std::shared_ptr<MSceneComponent> Component = *static_cast<std::shared_ptr<MSceneComponent>*>(Data);
                assert(Component);

                for (auto WeakChildComp : Component->GetChildComponents())
                {
                    std::shared_ptr<MSceneComponent> ChildComp = WeakChildComp.lock();
                    assert(ChildComp);

                    FAttachData AttachData = {};
                    AttachData.ParentName = Component->GetName();
                    AttachData.ChildName = ChildComp->GetName();
                    AttachDatas.push_back(AttachData);
                }
            }
        }

        TypeDesc = TypeDesc->Parent;
    }
}

void MActor::OnLoaded()
{
    Super::OnLoaded();

    uint32 NameCounter = 0;

    const FTypeDesc* TypeDesc = GetTypeDesc();
    while (TypeDesc != nullptr)
    {
        for (FPropertyDesc* PropertyDesc : TypeDesc->Properties)
        {
            const FTypeDesc* PropTypeDesc = PropertyDesc->TypeDesc;
            if (PropertyDesc == nullptr)
            {
                continue;
            }

            if (PropTypeDesc->IsA<MComponent>() == false)
            {
                continue;
            }

            void* Data = PropertyDesc->GetAsVoid(this);
            if (Data == nullptr)
            {
                continue;
            }

            if (PropertyDesc->IsA<MSceneComponent>()) // 씬 컴포넌트
            {
                std::shared_ptr<MSceneComponent> Component = *static_cast<std::shared_ptr<MSceneComponent>*>(Data);
                
                std::wstring Name = Component->GetName();
                if (Name.empty())
                {
                    Name = TEXT("Componenet") + std::to_wstring(NameCounter++);
                }

                AddComponent(Name, Component);
            }
            else // 일반 컴포넌트
            {

            }
        }

        TypeDesc = TypeDesc->Parent;
    }

    for (auto& AttachData : AttachDatas)
    {
        auto& Parent = getComponent(AttachData.ParentName);
        auto& Child = getComponent(AttachData.ChildName);

        if (Parent && Child)
        {
            Parent->AddChildComponent(Child);
        }
    }
}

void MActor::OnDuplicated(MObject* SrcObject)
{
    const FTypeDesc* TypeDesc = GetTypeDesc();
    while (TypeDesc != nullptr)
    {
        for (FPropertyDesc* PropertyDesc : TypeDesc->Properties)
        {
            void* Data = PropertyDesc->GetAsVoid(this);
            assert(Data);

            if (PropertyDesc->IsA<MSceneComponent>()) // 씬 컴포넌트
            {
                std::shared_ptr<MSceneComponent> Component = *static_cast<std::shared_ptr<MSceneComponent>*>(Data);
                AddComponent(Component->GetName(), Component);
            }
        }

        TypeDesc = TypeDesc->Parent;
    }

    for (auto& AttachData : AttachDatas)
    {
        auto& Parent = getComponent(AttachData.ParentName);
        auto& Child = getComponent(AttachData.ChildName);

        if (Parent && Child)
        {
            Parent->AddChildComponent(Child);
        }
    }
}

void MActor::RegistComponents()
{
    for (auto& [Name, Comp] : SceneComponents)
    {
        Comp->SetOwner(GetShared());
        Comp->setOwningActor(GetShared());
        RegisterComponent(Comp);
    }
}

void MActor::BeginPlay()
{
    OnBeganPlayDelegate.Broadcast(GetShared());

    for (auto& [Name, Comp] : SceneComponents)
    {
        Comp->BeginPlay();
    }

    bHasBegan = true;
}

MWorld* MActor::GetWorld()
{
    if (auto& Owner = GetOwner())
    {
        if(MWorld* OwningWorld = Owner->CastTo<MWorld>())
        {
            return OwningWorld;
        }
    }

    return nullptr;
}

void MActor::update(const Time deltaTime)
{
	tick(deltaTime);

	for (auto iter = SceneComponents.begin(); iter != SceneComponents.end(); ++iter)
	{
		if (false == iter->second->isUpdateable())
			continue;

		iter->second->Update(deltaTime);
	}

	for (auto iter = SceneComponents.begin(); iter != SceneComponents.end(); ++iter)
	{
		iter->second->OnUpdated();
	}
}

void MActor::tick(const Time deltaTime)
{
}

const Vec3 MActor::GetWorldTranslation()
{
    if (auto& RootComp = getComponent(ROOT_COMPONENT))
    {
        return RootComp->getWorldTranslation();
    }

    return VEC3ZERO;
}

void MActor::SetWorldTranslation(const Vec3& InTrans)
{
    if (auto& RootComp = getComponent(ROOT_COMPONENT))
    {
        RootComp->setTranslation(InTrans);
    }
}

std::shared_ptr<MSceneComponent>& MActor::getComponent(const wchar_t componentName[])
{
	return SceneComponents[componentName];
}

std::shared_ptr<MSceneComponent>& MActor::getComponent(const std::wstring& InName)
{
    return getComponent(InName.c_str());
}

bool MActor::AddComponent(const std::wstring& InName, std::shared_ptr<MSceneComponent> InComponent)
{
    return AddComponent(InName.c_str(), InComponent);
}

bool MActor::AddComponent(const wchar_t componentName[], std::shared_ptr<MSceneComponent> InComponent)
{
    if (InComponent == nullptr)
    {
        return false;
    }

    InComponent->SetName(componentName);

    for (auto& WeakChildComp : InComponent->GetChildComponents())
    {
        auto& ChildComp = WeakChildComp.lock();
        if (ChildComp == nullptr)
        {
            continue;
        }

        FAttachData NewAttachData = {};
        NewAttachData.ParentName = InComponent->GetName();
        NewAttachData.ChildName = ChildComp->GetName();
        AttachDatas.push_back(NewAttachData);
    }


	return MapUtility::FindInsert(SceneComponents, componentName, InComponent, true);
}