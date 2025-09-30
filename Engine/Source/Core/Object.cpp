#include "Object.h"
#include "Asset.h"

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
