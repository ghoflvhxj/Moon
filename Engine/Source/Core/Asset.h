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
    void Temp(const std::wstring& InPath);
    // 디스크에 저장된 애셋을 불러올 때 사용하는 함수.
    void LoadFromDisk(const std::wstring& InPath);
public:
    // 경로만 설정되었을 떄 이 함수를 이용해 로드할 수 있음. 각 애셋들은 적절하게 오버라이딩 해야함
    virtual bool Load();
    // 로드 성공 후 처리할 작업을 작성.
    virtual void OnLoaded();

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
