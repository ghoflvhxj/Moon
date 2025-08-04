#pragma once

#include "Include.h"

enum class EResourceType
{
    None,
    Texture,
    Mesh,
    Material,
    Animation,
    Shader,
    Count
};

class ENGINE_DLL MAsset : public std::enable_shared_from_this<MAsset>
{
public:
    MAsset() = default;
    MAsset(const std::wstring& InPath)
    {
        SetAssetPath(InPath);
    }
    virtual ~MAsset() = default;

protected:
    EResourceType ResourceType = EResourceType::None;

public:
    void SetAssetPath(const std::wstring& InPath);
    const std::wstring& GetAssetPath() const { return Path; }
protected:
    std::wstring Path;

    REFLECT_TOP(
        MAsset,
        PROPERTY(Path)
    )
};
