#pragma once

#include "Include.h"
#include "Core/Object.h"

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

class ENGINE_DLL MAsset : public MObject
{
public:
    MAsset() = default;
    MAsset(const std::wstring& InPath)
    {
        SetAssetPath(InPath);
    }
    virtual ~MAsset();

protected:
    EResourceType ResourceType = EResourceType::None;

public:
    virtual void LoadFromDisk(const std::wstring& InPath) override;
    virtual void OnLoaded() override;

public:
    void SetAssetPath(const std::wstring& InPath);
    const std::wstring& GetAssetPath() const { return Path; }
protected:
    std::wstring Path;

    REFLECT(
        MAsset,
        PROPERTY(Path)
    )
};
